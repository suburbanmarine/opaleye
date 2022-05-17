/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

var camPod = {};

function clickedRefreshButton(event) {
    console.log( "clickedRefreshButton" );

    camPod.xhttp_cam0_preview.open("GET", "/api/v1/cameras/cam0/live/thumb", true);
    camPod.xhttp_cam0_preview.responseType = 'blob';
    camPod.xhttp_cam0_preview.send()

    camPod.xhttp_cam1_preview.open("GET", "/api/v1/cameras/cam1/live/thumb", true);
    camPod.xhttp_cam1_preview.responseType = 'blob';
    camPod.xhttp_cam1_preview.send()
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
    camPod.jrpc.call('start_video_capture', ['pipe0']);
}
function btnVideoStopClick(event) {
    console.log( "btnVideoStopClick" );
    camPod.jrpc.call('stop_video_capture', ['pipe0']);
}

function btnRTPStreamStartClick(event) {
    console.log( "btnRTPStreamStartClick" );

    let client_ip = $('#client_ip').val();

    saveConfigToCookie();

    camPod.jrpc.call('start_rtp_stream', ['pipe0', client_ip, 5000]);
}
function btnRTPStreamStopClick(event) {
    console.log( "btnRTPStreamStopClick" );

    // let data = $('#ipform').serializeArray();
    // let client_ip = data['client_ip'];

    let client_ip = $('#client_ip').val();

    camPod.jrpc.call('stop_rtp_stream', ['pipe0', client_ip, 5000]);
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

    let exposure_setting = $('#exposure_setting_form_input').val();

    // ret = camPod.jrpc.call('set_camera_property_int', {camera_id: 'cam0', property_id: 'exposure_absolute', value: exposure_setting});
    ret = camPod.jrpc.call('set_camera_property_int', ['pipe0', 'cam0', 'exposure_absolute', parseInt(exposure_setting)]);
}
function btnSetExposureMode(event) {
    console.log( "btnSetExposureMode" );

    let exposure_mode = $('#exposure_mode_form_input').val();

    ret = camPod.jrpc.call('set_camera_property_int', ['pipe0', 'cam0', 'exposure_auto', parseInt(exposure_mode)]);
}
function btnSetFocusAbsolute(event) {
  console.log( "btnSetFocusAbsolute" );

  var val = $('#focus_absolute_form_input').val();

  ret = camPod.jrpc.call('set_camera_property_int', ['pipe0', 'cam0', 'focus_absolute', parseInt(val)]);
}
function btnSetFocusAuto(event) {
  console.log( "btnSetFocusAuto" );

  var val = $('#focus_auto_form_input').val();

  ret = camPod.jrpc.call('set_camera_property_int', ['pipe0', 'cam0', 'focus_auto', parseInt(val)]);
}
function btnSetBrightness(event) {
  console.log( "btnSetBrightness" );

  var val = $('#brightness_form_input').val();

  ret = camPod.jrpc.call('set_camera_property_int', ['pipe0', 'cam0', 'brightness', parseInt(val)]);
}
function btnSetGain(event) {
  console.log( "btnSetGain" );

  var val = $('#gain_form_input').val();

  ret = camPod.jrpc.call('set_camera_property_int', ['pipe0', 'cam0', 'gain', parseInt(val)]);
}

function refreshSensorData(event) {
    console.log( "refreshSensorData" );

    camPod.xhttp_sensor_preview.open("GET", "/api/v1/sensors", true);
    camPod.xhttp_sensor_preview.responseType = 'text';
    camPod.xhttp_sensor_preview.send()

    // $("#sensor_frame").attr('src', $("#sensor_frame").attr('src'));
}

function loadConfigFromCookie() {
    $('#client_ip').val(Cookies.get("client_ip"));

    $('#cbAutoRefresh').prop('checked', Cookies.get("cbAutoRefresh"));

    let test = JSON.parse(Cookies.get("test"));
}

function saveConfigToCookie() {

    Cookies.set("client_ip", $('#client_ip').val());

    Cookies.set("cbAutoRefresh", $('#cbAutoRefresh').is(':checked'));

    let test = {a: "A", b: "B"};
    Cookies.set("test", JSON.stringify(test));
}

function saveDefaultConfigToCookie() {

    Cookies.set("client_ip", '127.0.0.1');

    Cookies.set("cbAutoRefresh", true);

    let test = {a: "A", b: "B"};
    Cookies.set("test", JSON.stringify(test));
}

function isConfigValid() {
  let ret =        jQuery.isEmptyObject(Cookies.get("client_ip"    ));
      ret = ret || jQuery.isEmptyObject(Cookies.get("cbAutoRefresh"));

  return ! ret;
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
  $("#btnSetExposureAbsolute").on("click", btnSetExposureAbsolute );
  $("#btnSetExposureMode"    ).on("click", btnSetExposureMode     );
  $("#cbAdvancedMode"        ).on("click", clickedAdvancedMode    );
  $("#btnSetFocusAbsolute"   ).on("click", btnSetFocusAbsolute    );
  $("#btnSetFocusAuto"       ).on("click", btnSetFocusAuto        );
  $("#btnSetBrightness"      ).on("click", btnSetBrightness       );
  $("#btnSetGain"            ).on("click", btnSetGain             );

  // Event handlers
  camPod.refreshSensorDataIval = setInterval(refreshSensorData, 2000);

  camPod.xhttp_sensor_preview = new XMLHttpRequest();
  camPod.xhttp_sensor_preview.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
          var iframe_contentDocument = $("#sensor_frame").get(0).contentDocument
          iframe_contentDocument.open()
          iframe_contentDocument.write(this.responseText)
          iframe_contentDocument.close()
      }
  };

  camPod.xhttp_cam0_preview = new XMLHttpRequest();
  camPod.xhttp_cam0_preview.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
          var new_thumb     = new Blob([this.response], {type: 'image/jpg'});
          var new_thumb_url = window.URL.createObjectURL(this.response);
          
          window.URL.revokeObjectURL($("#cam0").attr('src'))
          $("#cam0").attr('src', new_thumb_url);
      }
  };
  
  camPod.xhttp_cam1_preview = new XMLHttpRequest();
  camPod.xhttp_cam1_preview.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
          var new_thumb     = new Blob([this.response], {type: 'image/jpg'});
          var new_thumb_url = window.URL.createObjectURL(this.response);

          window.URL.revokeObjectURL($("#cam1").attr('src'))
          $("#cam1").attr('src', new_thumb_url);
      }
  };

  // Configuration
  if(isConfigValid())
  {
    loadConfigFromCookie();
  }
  else
  {
    saveDefaultConfigToCookie();
    loadConfigFromCookie();
  }
}

$(document).ready(handleDocumentReady);
