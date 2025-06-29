from flask import Flask, request, jsonify, send_file, render_template
from flask_cors import CORS
import os
import uuid
import subprocess
import threading
import time
import json
from werkzeug.utils import secure_filename
import cv2
import tempfile
import shutil

app = Flask(__name__)
CORS(app)

# Configuration
UPLOAD_FOLDER = 'uploads'
OUTPUT_FOLDER = 'outputs'
TEMP_FOLDER = 'temp'
ALLOWED_EXTENSIONS = {'mp4', 'avi', 'mov', 'mkv', 'mk4'}

# Create directories
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
os.makedirs(OUTPUT_FOLDER, exist_ok=True)
os.makedirs(TEMP_FOLDER, exist_ok=True)

# Global task storage
tasks = {}

def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

def get_video_info(video_path):
    """Get basic video information"""
    try:
        cap = cv2.VideoCapture(video_path)
        if not cap.isOpened():
            return None
        
        fps = cap.get(cv2.CAP_PROP_FPS)
        frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        duration = frame_count / fps if fps > 0 else 0
        
        cap.release()
        
        return {
            'fps': round(fps, 2),
            'frame_count': frame_count,
            'width': width,
            'height': height,
            'duration': round(duration, 2)
        }
    except Exception as e:
        print(f"Error getting video info: {e}")
        return None

def preprocess_video(video_path, output_path):
    """Preprocess video for better processing"""
    try:
        cap = cv2.VideoCapture(video_path)
        if not cap.isOpened():
            return False
        
        # Get original properties
        fps = cap.get(cv2.CAP_PROP_FPS)
        frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        
        # Calculate new properties for optimization
        target_fps = min(fps, 15)  # Max 15 FPS for processing
        frame_skip = max(1, int(fps / target_fps))
        
        # Reduce resolution if too large
        scale_factor = 1.0
        if width > 1920 or height > 1080:
            scale_factor = min(1920 / width, 1080 / height)
            new_width = int(width * scale_factor)
            new_height = int(height * scale_factor)
        else:
            new_width, new_height = width, height
        
        # Setup video writer
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        out = cv2.VideoWriter(output_path, fourcc, target_fps, (new_width, new_height))
        
        frame_idx = 0
        processed_frames = 0
        
        print(f"Preprocessing video: {frame_count} frames -> {frame_count // frame_skip} frames")
        print(f"Resolution: {width}x{height} -> {new_width}x{new_height}")
        print(f"FPS: {fps} -> {target_fps}")
        
        while True:
            ret, frame = cap.read()
            if not ret:
                break
            
            # Skip frames for faster processing
            if frame_idx % frame_skip == 0:
                # Resize if needed
                if scale_factor != 1.0:
                    frame = cv2.resize(frame, (new_width, new_height))
                
                out.write(frame)
                processed_frames += 1
                
                # Progress update every 100 frames
                if processed_frames % 100 == 0:
                    progress = (frame_idx / frame_count) * 100
                    print(f"Preprocessing progress: {progress:.1f}%")
            
            frame_idx += 1
        
        cap.release()
        out.release()
        
        print(f"Preprocessing completed: {processed_frames} frames processed")
        return True
        
    except Exception as e:
        print(f"Error preprocessing video: {e}")
        return False

def process_video_task(task_id, video_path, output_path, parameters):
    """Process video in background thread"""
    try:
        tasks[task_id]['status'] = 'processing'
        tasks[task_id]['progress'] = 0
        tasks[task_id]['error'] = None
        
        # Get video info for progress calculation
        video_info = get_video_info(video_path)
        if video_info:
            tasks[task_id]['video_info'] = video_info
            print(f"Video info: {video_info['duration']}s, {video_info['frame_count']} frames")
        
        # Prepare C++ tracker command with real-time optimizations
        tracker_path = os.path.join('..', 'build', 'advanced_car_tracker')
        
        # Build command with real-time parameters
        cmd = [
            tracker_path,
            '-i', video_path,
            '-o', output_path
        ]
        
        # Add real-time mode if enabled
        if parameters.get('enable_realtime', True):
            cmd.extend(['--realtime-mode'])
        
        # Add advanced parameters if using advanced tracker
        if parameters.get('enable_occlusion', True):
            cmd.extend(['--occlusion-threshold', str(parameters['occlusion_threshold'])])
        if parameters.get('enable_reid', True):
            cmd.extend(['--reid-threshold', str(parameters['reid_threshold'])])
        if parameters.get('enable_camera', True):
            cmd.extend(['--camera-sensitivity', str(parameters['camera_sensitivity'])])
        
        # Add performance parameters
        cmd.extend(['--frame-skip', str(parameters['frame_skip'])])
        cmd.extend(['--resolution-scale', str(parameters['resolution_scale'])])
        
        # Override with automatic optimizations for very long videos
        if video_info and video_info['duration'] > 600:  # Longer than 10 minutes
            cmd.extend(['--frame-skip', '3'])
            cmd.extend(['--resolution-scale', '0.5'])
        elif video_info and video_info['duration'] > 300:  # Longer than 5 minutes
            cmd.extend(['--frame-skip', '2'])
            cmd.extend(['--resolution-scale', '0.7'])
        
        print(f"Running command: {' '.join(cmd)}")
        
        # Run the tracker
        start_time = time.time()
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            cwd=os.path.join('..', 'build')
        )
        
        # Monitor progress with real-time updates
        while process.poll() is None:
            # Simulate progress based on time elapsed
            elapsed = time.time() - start_time
            if video_info and video_info['duration'] > 0:
                # Real-time progress estimation
                if video_info['duration'] > 300:
                    estimated_total_time = video_info['duration'] * 0.3  # Very fast for real-time mode
                elif video_info['duration'] > 60:
                    estimated_total_time = video_info['duration'] * 0.5  # Fast for medium videos
                else:
                    estimated_total_time = video_info['duration'] * 1.0  # Real-time for short videos
                progress = min(95, int((elapsed / estimated_total_time) * 100))
                tasks[task_id]['progress'] = progress
            
            time.sleep(1)  # Check every second for real-time feel
        
        # Get final result
        stdout, stderr = process.communicate()
        end_time = time.time()
        
        if process.returncode == 0:
            tasks[task_id]['status'] = 'completed'
            tasks[task_id]['progress'] = 100
            tasks[task_id]['processing_time'] = round(end_time - start_time, 2)
            
            # Parse statistics from output (if available)
            statistics = parse_statistics(stdout)
            tasks[task_id]['statistics'] = statistics
            
            print(f"Task {task_id} completed successfully")
                
        else:
            tasks[task_id]['status'] = 'error'
            tasks[task_id]['error'] = stderr or 'Unknown error occurred'
            print(f"Task {task_id} failed: {stderr}")
            
    except Exception as e:
        tasks[task_id]['status'] = 'error'
        tasks[task_id]['error'] = str(e)
        print(f"Task {task_id} exception: {e}")

def parse_statistics(output):
    """Parse statistics from tracker output"""
    stats = {
        'processing_time': 0,
        'vehicles_detected': 0,
        'fps': 0,
        'accuracy': 0
    }
    
    try:
        lines = output.split('\n')
        for line in lines:
            if 'Total vehicles detected:' in line:
                stats['vehicles_detected'] = int(line.split(':')[1].strip())
            elif 'Average FPS:' in line:
                stats['fps'] = float(line.split(':')[1].strip())
            elif 'Average processing time:' in line:
                time_str = line.split(':')[1].strip()
                stats['processing_time'] = float(time_str.replace(' ms', ''))
            elif 'Accuracy:' in line:
                stats['accuracy'] = float(line.split(':')[1].strip().replace('%', ''))
    except:
        pass
    
    return stats

@app.route('/')
def index():
    """Serve the main page"""
    return render_template('index.html')

@app.route('/api/start', methods=['POST'])
def start_processing():
    """Start video processing"""
    try:
        # Check if video file is present
        if 'video' not in request.files:
            return jsonify({'success': False, 'error': 'No video file provided'}), 400
        
        video_file = request.files['video']
        if video_file.filename == '':
            return jsonify({'success': False, 'error': 'No video file selected'}), 400
        
        if not allowed_file(video_file.filename):
            return jsonify({'success': False, 'error': 'Invalid file type'}), 400
        
        # Generate unique task ID
        task_id = str(uuid.uuid4())
        
        # Save uploaded video
        filename = secure_filename(video_file.filename)
        video_path = os.path.join(UPLOAD_FOLDER, f"{task_id}_{filename}")
        video_file.save(video_path)
        
        # Prepare output path
        output_filename = f"tracked_{filename}"
        output_path = os.path.join(OUTPUT_FOLDER, f"{task_id}_{output_filename}")
        
        # Get parameters
        parameters = {
            'detection_threshold': float(request.form.get('detection_threshold', 0.5)),
            'occlusion_threshold': float(request.form.get('occlusion_threshold', 0.3)),
            'reid_threshold': float(request.form.get('reid_threshold', 0.7)),
            'camera_sensitivity': float(request.form.get('camera_sensitivity', 0.1)),
            'enable_realtime': request.form.get('enable_realtime', 'true').lower() == 'true',
            'enable_occlusion': request.form.get('enable_occlusion', 'true').lower() == 'true',
            'enable_reid': request.form.get('enable_reid', 'true').lower() == 'true',
            'enable_camera': request.form.get('enable_camera', 'true').lower() == 'true',
            'resolution_scale': float(request.form.get('resolution_scale', 1.0)),
            'frame_skip': int(request.form.get('frame_skip', 1))
        }
        
        # Initialize task
        tasks[task_id] = {
            'status': 'queued',
            'progress': 0,
            'video_path': video_path,
            'output_path': output_path,
            'parameters': parameters,
            'created_at': time.time(),
            'error': None,
            'statistics': {}
        }
        
        # Start processing in background thread
        thread = threading.Thread(
            target=process_video_task,
            args=(task_id, video_path, output_path, parameters)
        )
        thread.daemon = True
        thread.start()
        
        return jsonify({
            'success': True,
            'task_id': task_id,
            'message': 'Processing started'
        })
        
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/status/<task_id>')
def get_status(task_id):
    """Get processing status"""
    if task_id not in tasks:
        return jsonify({'success': False, 'error': 'Task not found'}), 404
    
    task = tasks[task_id]
    
    return jsonify({
        'success': True,
        'status': task['status'],
        'progress': task['progress'],
        'error': task['error'],
        'statistics': task.get('statistics', {}),
        'created_at': task['created_at']
    })

@app.route('/api/download/<task_id>')
def download_result(task_id):
    """Download processed video"""
    if task_id not in tasks:
        return jsonify({'success': False, 'error': 'Task not found'}), 404
    
    task = tasks[task_id]
    
    if task['status'] != 'completed':
        return jsonify({'success': False, 'error': 'Processing not completed'}), 400
    
    if not os.path.exists(task['output_path']):
        return jsonify({'success': False, 'error': 'Output file not found'}), 404
    
    try:
        return send_file(
            task['output_path'],
            as_attachment=True,
            download_name=os.path.basename(task['output_path'])
        )
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/tasks')
def list_tasks():
    """List all tasks"""
    task_list = []
    for task_id, task in tasks.items():
        task_list.append({
            'task_id': task_id,
            'status': task['status'],
            'progress': task['progress'],
            'created_at': task['created_at'],
            'filename': os.path.basename(task['video_path'])
        })
    
    return jsonify({
        'success': True,
        'tasks': task_list
    })

@app.route('/api/health')
def health_check():
    """Health check endpoint"""
    return jsonify({
        'success': True,
        'status': 'healthy',
        'timestamp': time.time()
    })

if __name__ == '__main__':
    print("🚗🚁 Car Chase Tracking System Backend")
    print("=====================================")
    print("Starting Flask server...")
    print("Frontend will be available at: http://localhost:8080")
    print("API endpoints:")
    print("  POST /api/start - Start video processing")
    print("  GET  /api/status/<task_id> - Get processing status")
    print("  GET  /api/download/<task_id> - Download result")
    print("  GET  /api/tasks - List all tasks")
    print("  GET  /api/health - Health check")
    print()
    
    app.run(debug=True, host='0.0.0.0', port=8080) 