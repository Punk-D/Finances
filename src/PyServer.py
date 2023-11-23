from flask import Flask, make_response, request, jsonify
import threading
import queue
import socket
import json

app = Flask(__name__)

# Request queue for sending tasks to C++ module
request_queue = queue.Queue()
# Shared dictionary to store request-response associations
request_responses = {}

cpp_server_address = ('localhost', 12345)  # Adjust the address and port

@app.route('/', methods=['POST'])
def handle_request():
    content_type = request.headers.get('Content-Type')
    print(f"Content-Type: {content_type}")

    if content_type == 'application/json':
        # If it's a JSON request, print the JSON data
        request_data = request.get_json()
        print("JSON Request Data:")
        print(request_data)
    else:
        # Handle other content types if needed
        print("Unsupported Content-Type")
    request_data = request.get_json()

    # Generate a unique request ID (you can use a more robust method)
    request_id = len(request_responses) + 1

    # Store the request and its ID in the dictionary
    request_responses[request_id] = None

    print("Request received")

    # Put the request in the queue
    request_queue.put((request_id, request_data))

    print("Request sent for processing")

    # Wait for the response to be updated
    while request_responses[request_id] is None:
        pass

    print("Request processing result received")

    # Retrieve the response and remove it from the dictionary
    response = request_responses.pop(request_id)

    print("Request processing result sent to client")

    # Create an HTTP response with "Content-Type: application/json"
    response_json = jsonify({'Response': response})
    response_json.headers['Content-Type'] = 'application/json'

    # Use make_response to set the "Content-Type" header
    response_with_header = make_response(response_json)

    return response_with_header

# Worker thread to process requests
def worker():
    while True:
        request_id, request_data = request_queue.get()
        # Convert the request data to a JSON string
        request_json = json.dumps(request_data)
        # Connect to the C++ module and send the request
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(cpp_server_address)
            s.sendall(request_json.encode())
            response = s.recv(1024).decode()  # Adjust the buffer size if needed
        request_responses[request_id] = response

if __name__ == '__main__':
    # Start the worker thread
    worker_thread = threading.Thread(target=worker)
    worker_thread.daemon = True
    worker_thread.start()

    # Start the Flask app with multithreading support
    app.run(host='localhost', port=8080, threaded=True)
