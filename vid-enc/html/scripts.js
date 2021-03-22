// function clickedStartButton() {
//   img = document.getElementById("cam0");

//   img.src="captcha-demo.php?rand_number=" + Math.random();
// }
// function clickedStopButton() {
//   img = document.getElementById("cam0");

//   img.src="captcha-demo.php?rand_number=" + Math.random(); 
// }

function clickedRefreshButton(event) {
    console.log( "clickedRefreshButton" );

    var url = $("#cam0").attr("src");
    $("#cam0").removeAttr("src").attr("src", url);
}
function clickedStartButton(event) {
    console.log( "clickedStartButton" );
     var jrpc = simple_jsonrpc.connect_xhr('http://127.0.0.1/api/v1');
     jrpc.call('start', ['cam0']);
}
function clickedStopButton(event) {
    console.log( "clickedStopButton" );
    var jrpc = simple_jsonrpc.connect_xhr('http://127.0.0.1/api/v1');
    jrpc.call('stop', ['cam0']);
}

function handleDocumentReady(jQuery) {
  $("#btnRefresh").on("click", clickedRefreshButton );
  $("#btnStart"  ).on("click", clickedStartButton   );
  $("#btnStop"   ).on("click", clickedStopButton    );
}

$(document).ready(handleDocumentReady);
