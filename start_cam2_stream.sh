curl -X POST \
     -H 'Content-Type: application/json' \
     -d '{"jsonrpc":"2.0","id":100,"method":"start_rtp_stream","params":["pipe2", "192.168.5.54", 5004]}' \
     http://192.168.5.7:80/api/v1
