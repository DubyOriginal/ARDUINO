

//*******************************************************************************************************
/*
    0 : WL_IDLE_STATUS when Wi-Fi is in process of changing between statuses
    1 : WL_NO_SSID_AVAILin case configured SSID cannot be reached
    3 : WL_CONNECTED after successful connection is established
    4 : WL_CONNECT_FAILED if password is incorrect
    6 : WL_DISCONNECTED if module is not configured in station mode
*/
String decodeWifiSTATUS(int wifiStatus){
  switch(wifiStatus){
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS"; 
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL"; 
    case WL_CONNECTED:
      return "WL_CONNECTED"; 
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED"; 
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED"; 
    default:
      return "UNKNOWN_STATUS"; 
  }
}

//*******************************************************************************************************
//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}


//*******************************************************************************************************
