Pebble.addEventListener("showConfiguration", function(e) {
  console.log("Showing configuration");
  Pebble.openURL("http://julia/pebble/config.html");
});

Pebble.addEventListener("webviewclosed", function(e) {
  var config = JSON.parse(decodeURIComponent(e.response));
  console.log("Returned from config");
  if (config.isCancel) {
    console.log("Cancelled configuration");
    return;
  }
  console.log("Sending message to the app: " + JSON.stringify(config));
  Pebble.sendAppMessage(config);
  console.log("Sent");
});

Pebble.addEventListener("ready", function(e) {
  console.log("Launching app");
  localStorage.config = JSON.stringify({
      alert53: "false"
  }); 
  if (localStorage.config != undefined) {
    console.log("Sending message to the app: " + localStorage.config);
    Pebble.sendAppMessage(JSON.parse(localStorage.config));
    console.log("Message sent");
  } else {
    console.log("No config to send, resetting config");
    localStorage.config = JSON.stringify({
        alert53: "false"
    });
  }
});