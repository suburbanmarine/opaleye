curl -X POST \
     -H 'Content-Type: application/json' \
     -d '{"jsonrpc":"2.0","id":100,"method":"start_rtp_stream","params":["cam1", "192.168.21.20", 5002]}' \
     http://192.168.40.70:80/api/v1
