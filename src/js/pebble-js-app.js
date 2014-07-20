Pebble.addEventListener("showConfiguration",
  function(e) {
    console.log("Showing configuration");
    Pebble.openURL("http://julia/config.html");
  }
);

Pebble.addEventListener("configurationClosed",
  function(e) {
    console.log("Configuration window returned: " + e.response);
  }
);
