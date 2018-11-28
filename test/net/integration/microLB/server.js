var http = require('http');

var dataString = function() {
  var len = 1024*1024 * 50;
  return '#'.repeat(len);
}

var stringToColour = function(str) {
  var hash = 0;
  for (var i = 0; i < str.length; i++) {
    hash = str.charCodeAt(i) + ((hash << 5) - hash);
  }
  var colour = '#';
  for (var i = 0; i < 3; i++) {
    var value = (hash >> (i * 8)) & 0xFF;
    colour += ('00' + value.toString(16)).substr(-2);
  }
  return colour;
}

//We need a function which handles requests and send response
function handleRequest(request, response){
  response.setTimeout(500);
  var addr = request.connection.localPort;
  response.end(addr.toString() + dataString());
}

http.createServer(handleRequest).listen(6001, '10.0.0.1');
http.createServer(handleRequest).listen(6002, '10.0.0.1');
http.createServer(handleRequest).listen(6003, '10.0.0.1');
http.createServer(handleRequest).listen(6004, '10.0.0.1');
