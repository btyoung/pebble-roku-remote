// Listen for when an AppMessage is received
var IP = '192.168.0.146';

Pebble.addEventListener('appmessage',
  function(e) {
    var key = e.payload.dummy;
    var url = 'http://' + IP + ':8060/keypress/' + key;
    
    console.log('Sending message ' + url);
    var xhr = new XMLHttpRequest();
    xhr.open('POST', url);
    xhr.send();
  }                     
);
