var camPod = {};

function clickedRefreshButton(event) {
    console.log( "clickedRefreshButton" );

    var url = $("#cam0").attr("src");
    $("#cam0").removeAttr("src").attr("src", url);
}
function clickedStartButton(event) {
    console.log( "btnGstStart" );
    camPod.jrpc.call('start', ['cam0']);
}
function clickedStopButton(event) {
    console.log( "btnGstStart" );
    camPod.jrpc.call('stop', ['cam0']);
}

function btnStillStartClick(event) {
    console.log( "btnStillStartClick" );
    camPod.jrpc.call('start_still_capture', ['cam0']);
}
function btnStillStopClick(event) {
    console.log( "btnStillStopClick" );
    camPod.jrpc.call('stop_still_capture', ['cam0']);
}

function btnVideoStartClick(event) {
    console.log( "btnVideoStartClick" );
    camPod.jrpc.call('start_video_capture', ['cam0']);
}
function btnVideoStopClick(event) {
    console.log( "btnVideoStopClick" );
    camPod.jrpc.call('stop_video_capture', ['cam0']);
}

function btnRTPStreamStartClick(event) {
    console.log( "btnRTPStreamStartClick" );
    camPod.jrpc.call('start_rtp_stream', ['127.0.0.1', 5000]);
}
function btnRTPStreamStopClick(event) {
    console.log( "btnRTPStreamStopClick" );
    camPod.jrpc.call('stop_rtp_stream', ['127.0.0.1', 5000]);
}
function btnGetPipelineStatus(event) {
   console.log( "btnGetPipelineStatus" );
   ret = camPod.jrpc.call('get_pipeline_status');
}
function btnGetPipelineGraph(event) {
   console.log( "btnGetPipelineGraph" );
   ret = camPod.jrpc.call('get_pipeline_graph');
}
function handleDocumentReady(jQuery) {
  
  var currentLocation = window.location;

  var rpcurl = 'http://' + currentLocation.hostname + '/api/v1';

  camPod.jrpc = simple_jsonrpc.connect_xhr(rpcurl);

  $("#btnRefresh"           ).on("click", clickedRefreshButton   );
  $("#btnStillStart"        ).on("click", btnStillStartClick     );
  $("#btnStillStop"         ).on("click", btnStillStopClick      );
  $("#btnVideoStart"        ).on("click", btnVideoStartClick     );
  $("#btnVideoStop"         ).on("click", btnVideoStopClick      );
  $("#btnRTPStreamStart"    ).on("click", btnRTPStreamStartClick );
  $("#btnRTPStreamStop"     ).on("click", btnRTPStreamStopClick  );
  $("#btnGetPipelineStatus" ).on("click", btnGetPipelineStatus   );
  $("#btnGetPipelineGraph"  ).on("click", btnGetPipelineGraph    );
}

$(document).ready(handleDocumentReady);
