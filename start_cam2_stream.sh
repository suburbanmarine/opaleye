curl -X POST \
     -H 'Content-Type: application/json' \
     -d '{"jsonrpc":"2.0","id":100,"method":"start_rtp_stream","params":["cam0", "192.168.21.20", 5004]}' \
     http://192.168.40.71:80/api/v1
