#include "http_server.h"

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

time_t last_catch;

httpd_handle_t camera_httpd = NULL;

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

static esp_err_t capture_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
#ifdef USE_SERIAL_DEBUG
        Serial.println("Camera capture failed");
#endif
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    size_t out_len, out_width, out_height;
    uint8_t * out_buf;
    bool s;
    bool detected = false;
    int face_id = 0;
    if(fb->width > 400){
        size_t fb_len = 0;
        if(fb->format == PIXFORMAT_JPEG){
            fb_len = fb->len;
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        } else {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
            fb_len = jchunk.len;
        }
        esp_camera_fb_return(fb);
        int64_t fr_end = esp_timer_get_time();
        // Serial.printf("JPG: %uB %ums\n", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start)/1000));
        return res;
    }

    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
    if (!image_matrix) {
        esp_camera_fb_return(fb);
#ifdef USE_SERIAL_DEBUG
        Serial.println("dl_matrix3du_alloc failed");
#endif
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    out_buf = image_matrix->item;
    out_len = fb->width * fb->height * 3;
    out_width = fb->width;
    out_height = fb->height;

    s = fmt2rgb888(fb->buf, fb->len, fb->format, out_buf);
    esp_camera_fb_return(fb);
    if(!s){
        dl_matrix3du_free(image_matrix);
#ifdef USE_SERIAL_DEBUG
        Serial.println("to rgb888 failed");
#endif
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    jpg_chunking_t jchunk = {req, 0};
    s = fmt2jpg_cb(out_buf, out_len, out_width, out_height, PIXFORMAT_RGB888, 90, jpg_encode_stream, &jchunk);
    dl_matrix3du_free(image_matrix);
    if(!s){
#ifdef USE_SERIAL_DEBUG
        Serial.println("JPEG compression failed");
#endif
        return ESP_FAIL;
    }

    return res;
}

static esp_err_t head_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  char direction[32] = {0,}, found[6] = {0,};
  char body[18] = {0,};
  
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
#ifdef USE_SERIAL_DEBUG
      Serial.println("head_handler : malloc failed");
#endif
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "dir", direction, sizeof(direction)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
      if (httpd_query_key_value(buf, "found", found, sizeof(found)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  if(!strcmp(found, "true")) {
//    mono_eye_leds.red(4);
//    mono_eye_leds.orange(2);
    time(&last_catch);
  }

  sensor_t * s = esp_camera_sensor_get();
  //flip the camera vertically
  //s->set_vflip(s, 1);          // 0 = disable , 1 = enable
  // mirror effect
  //s->set_hmirror(s, 1);          // 0 = disable , 1 = enable

  if(!strcmp(direction, "left")) {
    cmdSerial.printf("Left");
    cmdSerial.printf(COMMAND_DELIMETER);
    cmdSerial.flush();
#ifdef USE_SERIAL_DEBUG
    Serial.println("Left");
#endif
  }
  else if(!strcmp(direction, "right")) {
    cmdSerial.printf("Right");
    cmdSerial.printf(COMMAND_DELIMETER);
    cmdSerial.flush();
#ifdef USE_SERIAL_DEBUG
    Serial.println("Right");
#endif
  }
  else {
#ifdef USE_SERIAL_DEBUG
    Serial.print("Wrong direction : ");
    Serial.println(direction);
#endif
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, body, strlen(body));
}

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
#ifdef USE_SERIAL_DEBUG
    Serial.printf("Camera init failed with error 0x%x", err);
#endif
    return;
  }
}

esp_err_t wifiEventHandler(void* userParameter, system_event_t *event) {
    switch(event->event_id){
    case SYSTEM_EVENT_AP_STACONNECTED:
        #ifdef USE_SERIAL_DEBUG
        Serial.println("AP_STACONNECTED");
        #endif
        startCameraServer();
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        #ifdef USE_SERIAL_DEBUG
        Serial.println("AP_STADISCONNECTED");
        #endif
        stopCameraServer();
        break;
    default:
        #ifdef USE_SERIAL_DEBUG
        Serial.print("WiFi evevnt ");
        Serial.println(event->event_id);
        #endif
        break;
    }
    return ESP_OK;
}

void initCameraServer() {
    tcpip_adapter_init();
    tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
    tcpip_adapter_ip_info_t ipAddressInfo;
    memset(&ipAddressInfo, 0, sizeof(ipAddressInfo));
    IP4_ADDR(
        &ipAddressInfo.ip,
        SOFT_AP_IP_ADDRESS_1,
        SOFT_AP_IP_ADDRESS_2,
        SOFT_AP_IP_ADDRESS_3,
        SOFT_AP_IP_ADDRESS_4);
    IP4_ADDR(
        &ipAddressInfo.gw,
        SOFT_AP_GW_ADDRESS_1,
        SOFT_AP_GW_ADDRESS_2,
        SOFT_AP_GW_ADDRESS_3,
        SOFT_AP_GW_ADDRESS_4);
    IP4_ADDR(
        &ipAddressInfo.netmask,
        SOFT_AP_NM_ADDRESS_1,
        SOFT_AP_NM_ADDRESS_2,
        SOFT_AP_NM_ADDRESS_3,
        SOFT_AP_NM_ADDRESS_4);
    tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ipAddressInfo);
    tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
    esp_event_loop_init(wifiEventHandler, NULL);
    wifi_init_config_t wifiConfiguration = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifiConfiguration);
    esp_wifi_set_mode(WIFI_MODE_AP);

    wifi_config_t apConfiguration;
    strcpy((char*)apConfiguration.ap.ssid, (char *)SOFT_AP_SSID);
    apConfiguration.ap.ssid_len = 0;
    apConfiguration.ap.ssid_hidden = 0;
    apConfiguration.ap.beacon_interval = 150;

    esp_wifi_set_config(WIFI_IF_AP, &apConfiguration);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_start();
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t head_uri = {
    .uri       = "/motor",
    .method    = HTTP_GET,
    .handler   = head_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t capture_uri = {
    .uri       = "/capture",
    .method    = HTTP_GET,
    .handler   = capture_handler,
    .user_ctx  = NULL
  };
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &head_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
  }
}

void stopCameraServer(){
      if(camera_httpd != NULL){
        httpd_stop(camera_httpd);
    }
}
