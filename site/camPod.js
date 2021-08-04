var camPod = {};

function clickedRefreshButton(event) {
    console.log( "clickedRefreshButton" );

    // var url = $("#cam0").attr("src");
    // $("#cam0").removeAttr("src").attr("src", url);

    $("#cam0").attr('src', $("#cam0").attr('src'));
}
function clickedAutoRefreshCb(event) {
    console.log( "clickedAutoRefreshCb" );

    let isChecked = $("#cbAutoRefresh").is(":checked");
    if(isChecked)
    {
      camPod.cbAutoRefreshIval = setInterval(clickedRefreshButton, 2000);
    }
    else
    {
      clearInterval(camPod.cbAutoRefreshIval);
    }
}
function clickedAdvancedMode(event) {
    console.log( "clickedAdvancedMode" );

    let isChecked = $("#cbAdvancedMode").is(":checked");
    if(isChecked)
    {
      $("#debug").css("display", "");
    }
    else
    {
      $("#debug").css("display", "none");
    }
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

    // let data = $('#ipform').serializeArray();
    // let client_ip = data['client_ip'];

    saveConfigToCookie();

    camPod.jrpc.call('start_rtp_stream', [client_ip, 5000]);
}
function btnRTPStreamStopClick(event) {
    console.log( "btnRTPStreamStopClick" );

    // let data = $('#ipform').serializeArray();
    // let client_ip = data['client_ip'];

    let client_ip = $('#client_ip').val();

    camPod.jrpc.call('stop_rtp_stream', [client_ip, 5000]);
}
function btnGetPipelineStatus(event) {
    console.log( "btnGetPipelineStatus" );
    let ret = camPod.jrpc.call('get_pipeline_status');
}
function btnGetPipelineGraph(event) {
    console.log( "btnGetPipelineGraph" );
    letret = camPod.jrpc.call('get_pipeline_graph');
}

function btnSetExposureAbsolute(event) {
    console.log( "btnSetExposureAbsolute" );


    var exposure_setting = $('#exposure_setting_form_input').val();


    // ret = camPod.jrpc.call('set_camera_property', {camera_id: 'cam0', property_id: 'exposure_absolute', value: exposure_setting});
    ret = camPod.jrpc.call('set_camera_property', ['cam0', 'exposure_absolute', parseInt(exposure_setting)]);
}
function btnSetExposureMode(event) {
    console.log( "btnSetExposureMode" );

    var exposure_mode = $('#exposure_mode_form_input').val();

    // ret = camPod.jrpc.call('set_camera_property', {camera_id: 'cam0', property_id: 'exposure_mode', value: exposure_mode});
    ret = camPod.jrpc.call('set_camera_property', ['cam0', 'exposure_mode', parseInt(exposure_mode)]);
}

function refreshSensorData(event) {
    console.log( "refreshSensorData" );

    $("#sensor_frame").attr('src', $("#sensor_frame").attr('src'));
}

function loadConfigFromCookie() {
    $('#client_ip').val(Cookies.get("client_ip"));

    $('#cbAutoRefresh').val(Cookies.get("cbAutoRefresh"));
}

function saveConfigToCookie() {

    Cookies.set("client_ip", $('#client_ip').val());

    Cookies.set("cbAutoRefresh", $('#cbAutoRefresh').val());

    let test = {a: "A", b: "B"};
    Cookies.set("test", JSON.stringify(test));
}

function handleDocumentReady(jQuery) {
  
  //RPC
  let currentLocation = window.location;
  let rpcurl = 'http://' + currentLocation.hostname + '/api/v1';
  camPod.jrpc = simple_jsonrpc.connect_xhr(rpcurl);

  //Callbacks
  $("#btnRefresh"            ).on("click", clickedRefreshButton   );
  $("#cbAutoRefresh"         ).on("click", clickedAutoRefreshCb   );
  $("#btnStillStart"         ).on("click", btnStillStartClick     );
  $("#btnStillStop"          ).on("click", btnStillStopClick      );
  $("#btnVideoStart"         ).on("click", btnVideoStartClick     );
  $("#btnVideoStop"          ).on("click", btnVideoStopClick      );
  $("#btnRTPStreamStart"     ).on("click", btnRTPStreamStartClick );
  $("#btnRTPStreamStop"      ).on("click", btnRTPStreamStopClick  );
  $("#btnGetPipelineStatus"  ).on("click", btnGetPipelineStatus   );
  $("#btnGetPipelineGraph"   ).on("click", btnGetPipelineGraph    );
  $("#btnSetExposureAbsolute").on("click", btnSetExposureAbsolute);
  $("#btnSetExposureMode"    ).on("click", btnSetExposureMode    );
  $("#cbAdvancedMode"        ).on("click", clickedAdvancedMode   );

  // Event handlers
  camPod.refreshSensorDataIval = setInterval(refreshSensorData, 2000);

  // Configuration
  loadConfigFromCookie();
}

$(document).ready(handleDocumentReady);
