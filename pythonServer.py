from flask import Flask, request
from datetime import datetime

app = Flask(__name__)

@app.route('/data', methods=['POST'])
def receive_data():
    data = request.get_json()
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    print(f"[{timestamp}] Dados recebidos: {data}")
    # Aqui podes guardar em CSV se quiseres
    return '', 200


@app.route('/ping', methods=['GET'])
def ping():
    return 'pong', 200    

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
