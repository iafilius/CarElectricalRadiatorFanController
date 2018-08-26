#ifdef USE_HTTP_SERVER

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//WiFiServer sync_server(80);
AsyncWebServer async_server(80);
#define SSE_reconnect_time_ms 5000
#define C_SSE_Update_throttle_time_ms  250
//const int C_SSE_Update_throttle_time_ms=250;    // cons int instead define to makeis visable from other modules

volatile bool g_Enforce_HTTP5_SSE_update = false;

// https://github.com/me-no-dev/ESPAsyncWebServer#async-event-source-plugin
//AsyncEventSource events("/events");     // html server events


void async_http_server_setup(void) {

  async_server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    char buffer[C_STR_BUFSIZE];
    String content = "<html><head><meta http-equiv=\"refresh\" content=\"5\" ></head><body>";

    content += "<h1>Welcome to uP fan Control monitor and setup</h1>";

    content += "Program Version: " + String(C_PROGVERSION) + "<BR>Schema: " + String(C_EEPROM_SCHEMA) + "<BR> Compiled: " + __DATE__ + " - " + __TIME__ + "<BR>";
    content += "g_CoolingWaterTemp_Motor_OUT_InC: " + String(g_CoolingWaterTemp_Motor_OUT_InC) + "<BR>";
    content += "g_CoolingWaterTemp_Motor_IN_InC: " + String(g_CoolingWaterTemp_Motor_IN_InC) + "<BR>";
    content += "dutycycle_desired: " + String(dutycycle_desired) + "<BR>";
    content += "g_PWM_Period (PWM) (Hz): " + String( (1.0 / g_PWM_Period) * 1000000) + "<BR>";
    content += "g_Temperature_PWM_Policy: " + String(g_Temperature_PWM_Policy) + "<BR>";
    content += "g_Temperature_PWM_Policy description: " + String(getPolicyDescription(g_Temperature_PWM_Policy)) + "<BR>";

    content += "Operating mode: ";
    if (g_operating_mode == C_OPERATING_MODE_AUTOMATIC) content += "Automatic<BR>";
    if (g_operating_mode == C_OPERATING_MODE_SIMULATED) content += "Simulated<BR>";
    if (g_operating_mode == C_OPERATING_MODE_MANUAL) content += "Manual<BR>";

    get_RunningTime(buffer,C_STR_BUFSIZE);
    content += "Running Time: " + String(buffer) + "<BR>";


    content += "<a href=\"/setup\">Setup</a><BR>";
    content += "<a href=\"/Cockpit\">Cockpit</a><BR>";
    content += "<a href=\"/debug\">Debug</a><BR>";
    content += "<a href=\"/update\">update firmware through http</a><BR>";
    content += "<a href=\"/\">Home</a><BR>";

    content += "</body></html>";

    request->send(200, "text/html", content);

    if (g_debug) {
      Serial.print("/ took so many bytes:");
      Serial.println(content.length());
      Serial.print("ESP.getFreeHeap: ");
      Serial.println(ESP.getFreeHeap());
    }

  });


  async_server.on("/Cockpit", HTTP_GET, [](AsyncWebServerRequest * request) {
    char buffer[C_STR_BUFSIZE];
    String content ;

#ifdef STRING_PLUS_IS_ALLOC_BUG_WORKAROUND
 #define Cockpitsize 4500
    content.reserve(Cockpitsize);     // This seems to reduce crashes a lot ( but tcp_close ... stays after )
#endif
    //<meta http-equiv=\"refresh\" content=\"5\" >\n


    content = "<!DOCTYPE html>\n\
<head>\n\
 <meta name=\"apple-mobile-web-app-capable\" content=\"yes\">\n\
 <meta charset=\"utf-8\">\n\
 <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, minimal-ui\">\n\
 <link rel=\"stylesheet\" href=\"/style.css\"  type=\"text/css\">\n\
 <title></title>\n\
</head>\n\
<body>\n\
";
    // if (!!window.EventSource) {\n

    content += "\n\
<script type=\"text/javascript\">\n\
if(typeof(EventSource) !== \"undefined\") {\n\
  var source = new EventSource('/events');\n\
\n\
    source.onmessage = function(event) {\n\
        //document.getElementById(\"result\").innerHTML = event.data ;\n\
        console.log(\"onmessage\", event.data);\n\
    };\n\
\n\
  source.addEventListener('open', function(event) {\n\
    console.log(\"Events Connected\");\n\
  }, false);\n\
\n\
  source.addEventListener('error', function(event) {\n\
    if (event.target.readyState != EventSource.OPEN) {\n\
      event.close();\n\
      reinitiateEventSource();\n\
      console.log(\"Events Disconnected\");\n\
    }\n\
  }, false);\n\
\n\
  source.addEventListener(\"radiatorvalue\", function(event) {\n\
    document.getElementById(\"radiatorvalue\").innerHTML = event.data ;\n\
    console.log(\"radiatorvalue\", event.data);\n\
  }, false);\n\
\n\
  source.addEventListener(\"motorvalue\", function(event) {\n\
    document.getElementById(\"motorvalue\").innerHTML = event.data ;\n\
    console.log(\"motorvalue\", event.data);\n\
  }, false);\n\
  source.addEventListener(\"dutycycle_actual\", function(event) {\n\
    document.getElementById(\"dutycycle_actual\").innerHTML = event.data ;\n\
    console.log(\"dutycycle_actual\", event.data);\n\
  }, false);\n\
  source.addEventListener(\"pwmunit\", function(event) {\n\
    document.getElementById(\"pwmunit\").innerHTML = event.data ;\n\
    console.log(\"pwmunit\", event.data);\n\
  }, false);\n\
  source.addEventListener(\"uptime\", function(event) {\n\
    document.getElementById(\"uptime\").innerHTML = event.data ;\n\
    console.log(\"uptime\", event.data);\n\
  }, false);\n\
  source.addEventListener(\"policy\", function(event) {\n\
    document.getElementById(\"policy\").innerHTML = event.data ;\n\
    console.log(\"policy\", event.data);\n\
  }, false);\n\
  source.addEventListener(\"control\", function(event) {\n\
    document.getElementById(\"control\").innerHTML = event.data ;\n\
    console.log(\"control\", event.data);\n\
  }, false);\n\
} else {\n\
  alert(\"Browser is crap ...no SSE!!\");\n\
}\n\
\n\
(function(l){var i,s={touchend:function(){}};for(i in s)l.addEventListener(i,s)})(document);  // IOS sticky hover fix\n\
\n\
function submitForm(params) {\n\
  var http = new XMLHttpRequest();\n\
  var url = '/setup/';\n\
  //var params = 'test=1&name=aap';\n\
\n\
  http.open('POST',url,true);\n\
\n\
  // Send proper header information along the request\n\
  http.setRequestHeader('Content-type','application/x-www-form-urlencoded');\n\
\n\
  http.onreadystatechange = function(){\n\
    // call function when the stte changes\n\
    if(http.readyState == 4 && http.status==200) {\n\
    alert(http.responseText);\n\
    }\n\
  }\n\
  http.send(params);\n\
}\n\
\n\
</script>\n\
";

    get_RunningTime(buffer, C_STR_BUFSIZE);
    //" + String(g_CoolingWaterTemp_Motor_OUT_InC) +  "
    // " + String(g_CoolingWaterTemp_Motor_IN_InC) + "
    //String(__dutycycle_actual)
    //" + String(dutycycle_desired) + "<BR>/1024
    // +String(buffer)  +
    //String(C_Temperature_PWM_Policy)

    content += "\n\
      <!--<div id=\"result\" name=\"ticker\"></div>-->\n\
      <div id=\"_motorvalue\" name=\"motor_value\"></div>\n\
      <div id=\"_radiatorvalue\" name=\"radiator_value\"></div>\n\
\n\
      <div class=\"grid\">\n\
        <div class=\"motortitle\">Motor<BR>Temperature</div>\n\
        <div id=\"motorvalue\" class=\"motorvalue\"></div>\n\
        <div class=\"motorunit\">°C</div>\n\
        <div class=\"radiatortitle\">Radiator<BR>Temperature</div>\n\
        <div id=\"radiatorvalue\" class=\"radiatorvalue\"></div>\n\
        <div class=\"radiatorunit\">°C</div>\n\
        <div class=\"pwmtitle\">PWM</div>\n\
        <div id=\"dutycycle_actual\" class=\"pwmvalue\"></div>\n\
        <div id=\"pwmunit\" class=\"pwmunit\"></div>\n\
        <div id=\"uptime\" class=\"uptime\"></div>\n\
        <div id=\"policy2\" class=\"policy\">\n\
          <button class=\"dropbtn\">\n\
          <div id=\"policy\">Aaap noot Miez</div>\n\
          <i class=\"fa fa-caret-down\"></i>\n\
          </button>\n\
          <div class=\"dropdown-content\">\n\
";

      for (int p=0 ; p < NR_CFunctions(); p++) {
        //content += "<input type='radio' name=coolingpolicy value='" + String(p) +  "'";
        content += "<a href=\"javascript:void(0);\" onclick=\"submitForm('api=1&coolingpolicy=" + String(p) +  "');\">" + String(getPolicyDescription(p)) + "</a>\n";
         //if (g_Temperature_PWM_Policy == p) content += "checked";
         //content += "> " + String(getPolicyDescription(p)) + "<br>";     
      }

          
/*            <a href=\"javascript:void(0);\" onclick=\"submitForm('coolingpolicy=0');\">0</a>\n\
            <a href=\"javascript:void(0);\" onclick=\"submitForm('coolingpolicy=1');\">1</a>\n\
            <a href=\"javascript:void(0);\" onclick=\"submitForm('coolingpolicy=2');\">2</a>\n\
            <a href=\"javascript:void(0);\" onclick=\"submitForm('coolingpolicy=3');\">3</a>\n\ 
            <a href=\"/setup\">2</a>\n\
            <a href=\"#\">3</a>\n\
*/        content+="</div>\n\
        </div>\n\
        <div id=\"control\" class=\"control\"></div>\n\
\n\
    </div>\n\
</body>\n\
</html>\n\
";
    request->send(200, "text/html", content);

#ifdef STRING_PLUS_IS_ALLOC_BUG_WORKAROUND
    if (g_debug) {
      Serial.print("/Cockpit reserved so many bytes:");
      Serial.println((Cockpitsize));
      Serial.print("/Cockpit took so many bytes:");
      Serial.println(content.length());
      if (content.length() > Cockpitsize) Serial.println("Increse reservation() !!!");
      Serial.print("ESP.getFreeHeap: ");
      Serial.println(ESP.getFreeHeap());
    }
#endif

  });


  async_server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    String content ;

#ifdef STRING_PLUS_IS_ALLOC_BUG_WORKAROUND
 #define stylecsssize 3000
    content.reserve(stylecsssize);    // This seems to reduce crashes a lot (tcp close/heap corruption/....)
#endif
    content = "* {\
    margin: 0;\
    padding: 0;\
}\
.grid {\
    display: grid;\
    grid-template-columns: auto 1fr auto;\
    width: 100vw;\
    height: 100vh;\
    grid-template-areas:\
    \"motortitle motorvalue motorunit\"\
    \"radiatortitle radiatorvalue radiatorunit\"\
    \"pwmtitle pwmvalue pwmunit\"\
    \"uptime policy control\";\
}\
.motortitle {\
    background-color: rgb(10, 10, 10);\
    color: darkgray;\
    grid-area: motortitle;\
    font: 1em sans-serif;\
}\
.motorvalue {\
    background-color: rgb(10, 10, 10);\
    color: darkgray;\
    grid-area: motorvalue;\
    font: 10em sans-serif;\
    font-size: 15vw;\
    text-align: right;\
}\
.motorunit {\
    background-color: rgb(10, 10, 10);\
    color: darkgray;\
    grid-area: motorunit;\
    font: 2em sans-serif;\
}\
.radiatortitle {\
    background-color: rgb(15, 15, 15);\
    color: darkgray;\
    grid-area: radiatortitle;\
    font: 1em sans-serif;   \
}\
.radiatorvalue {\
    background-color: rgb(15, 15, 15);\
    color: darkgray;\
    grid-area: radiatorvalue;\
    font: 10em sans-serif;\
    font-size: 15vw;\
    text-align: right;\
    text-transform: uppercase;\
}\
.radiatorunit {\
    background-color: rgb(15, 15, 15);\
    color: darkgray;\
    grid-area: radiatorunit;\
    font: 2em sans-serif;\
}\
.pwmtitle {\
    background-color: rgb(10, 10, 10);\
    color: darkgray;\
    grid-area: pwmtitle;\
    font: 1em sans-serif;\
}\
.pwmvalue {\
    background-color: rgb(10, 10, 10);\
    color: darkgray;\
    grid-area: pwmvalue;\
    font: 10em sans-serif;\
    font-size: 15vw;\
    text-align: right;\
}\
.pwmunit {\
    background-color: rgb(10, 10, 10);\
    color: darkgray;\
    grid-area: pwmunit;\
    font: 2em sans-serif;\
}\
.uptime {\
    background-color: rgb(15, 15, 15);\
    color: darkgray;\
    grid-area: uptime;\
    font: 1em sans-serif;\
}\
.policy {\
    background-color: rgb(15, 15, 15);\
    color: darkgray;\
    grid-area: policy;\
    font: 1em sans-serif;\
}\
.policy:hover   .dropdown-content {\
  display: block;\
}\
.policy:hover .dropbtn {\
  background-color: #3e8e41;\
}\
.control {\
    background-color: rgb(15, 15, 15);\
    color: darkgray;\
    grid-area: control;\
    font: 1em sans-serif;\
}\
.dropbtn  {\
    background-color: rgb(15, 15, 15);\
    color: darkgray;\
    grid-area: policy;\
    font: 1em sans-serif;\
  display: inline-block;\
  position: relative;\
  border: 0px;\
}\
.dropdown-content {\
  display: none;\
    grid-area: policy;\
  position: absolute;\
  bottom: 0px;\
# position: relative;\
  background-color: #f1f1f1;\
  min-width: 160px;\
# box-shadow: 10px 18px 116px 10px rgba(0,0,0,0.8);\
# z-index: 1;\
# right: 0;\
}\
.dropdown-content a {\
  color: black;\
  padding: 12px 16px;\
  text-decoration: none;\
# box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.8);\
  display: block;\
}\
.dropdown-content a:hover {\
  background-color: #ddd;\
  transition: 1s;\
}\
";
    request->send(200, "text/css", content);

#ifdef STRING_PLUS_IS_ALLOC_BUG_WORKAROUND
    if (g_debug) {
      Serial.print("/style.css reserved so many bytes:");
      Serial.println(stylecsssize);
      Serial.print("/style.css took so many bytes:");
      Serial.println(content.length());
      if (content.length() > stylecsssize) Serial.println("Increse reservation() !!!");
      Serial.print("ESP.getFreeHeap: ");
      Serial.println(ESP.getFreeHeap());
    }
#endif

  });





  async_server.on("/debug", HTTP_GET, [](AsyncWebServerRequest * request) {
    String content = "text/plain Status Overview\n";

    Serial.println("/debug");

    // print the paramater via ?&aap=xx
    int paramsNr = request->params();
    Serial.println(paramsNr);

    for (int i = 0; i < paramsNr; i++) {
      AsyncWebParameter* p = request->getParam(i);
      Serial.print("Param name: ");
      Serial.println(p->name());
      Serial.print("Param value: ");
      Serial.println(p->value());
      Serial.println("------");
    }

    /*
        Serial.print("ESP.getFreeHeap: ");
        Serial.println(ESP.getFreeHeap());
    */

    content += "g_debug: " + String(g_debug) + "\n";
    content += "g_operating_mode: ";
    if ( g_operating_mode == C_OPERATING_MODE_AUTOMATIC) content += F("AUTOMATIC\n");
    if ( g_operating_mode == C_OPERATING_MODE_SIMULATED) content += F("SIMULATED\n");
    if ( g_operating_mode == C_OPERATING_MODE_MANUAL) content += F("MANUAL\n");


    content += "g_simulate_values[MotorTemp_IDX]: " + String(g_simulate_values[MotorTemp_IDX]) + "\n";
    content += "g_simulate_values[RadiatorTemp_IDX]: " + String(g_simulate_values[RadiatorTemp_IDX]) + "\n";

    // content += "Device 0[DallasDevices[MotorTemp_IDX]] Address: ")); printAddress(DallasDevices[MotorTemp_IDX]);
    // content += "Device 0[DallasDevices[MotorTemp_IDX]] Alarms: ")); printAlarms(DallasDevices[MotorTemp_IDX]);
    // content += "Device 1[DallasDevices[RadiatorTemp_IDX]] Address: ")); printAddress(DallasDevices[RadiatorTemp_IDX]);
    // content += "Device 1[DallasDevices[RadiatorTemp_IDX]] Alarms: ")); printAlarms(DallasDevices[RadiatorTemp_IDX]);

    content += "g_dallas_conversion_time(ms): " + String(g_dallas_conversion_time) + "\n";
    content += "g_Dallas_Resulution: " + String(g_Dallas_Resolution) + "\n";
    content += "C_Dallas_Resulution: " + String(C_Dallas_Resolution) + "\n";
    content += "actual global library resolution: " + String(sensors.getResolution()) + "\n";
    content += "Device[0] resolution: " + String(sensors.getResolution(DallasDevices[MotorTemp_IDX])) + "\n";
    content += "Device[1] resolution: " + String(sensors.getResolution(DallasDevices[RadiatorTemp_IDX])) + "\n";
    content += "g_Dallas_Synchronous: " + String(g_Dallas_Synchronous) + "\n";
    content += "dutycycle_desired: " + String(dutycycle_desired) + "\n";
    content += "C_PWM_Period_us (PWM) (uS): " + String(C_PWM_Period_us) + "\n";
    content += "C_PWM_Period_us (PWM) (Hz): " + String( (1.0 / C_PWM_Period_us) * 1000000) + "\n";
    content += "g_PWM_Period (PWM) (uS): " + String(g_PWM_Period) + "\n";
    content += "g_PWM_Period (PWM) (Hz): " + String( (1.0 / g_PWM_Period) * 1000000) + "\n";
    content += "C_FAN_SLOWSTART_TIME: (S)" + String(C_FAN_SLOWSTART_TIME) + "\n" ;
    content += "g_FAN_SLOWSTART_TIME: (S)" + String(g_FAN_SLOWSTART_TIME) + "\n";

    // derived values, no constant defined directly
    //content += "C_pwm_soft_change_period_us: (uS)"));
    //String(C_pwm_soft_change_period_us);

    // derived values, no constant defined directly
    //content += "C_pwm_soft_change_period_us (Hz): "));
    //String( (1.0/C_pwm_soft_change_period_us)*1000000);

    preferences.begin(C_Preferences_namespace, true);

    content += "g_pwm_control_timer_period_us: (uS)" + String(g_pwm_control_timer_period_us) + "\n";
    content += "g_pwm_control_timer_period_us (Hz): " + String( (1.0 / g_pwm_control_timer_period_us) * 1000000) + "\n";
    content += "g_pwm_soft_step:" + String(g_pwm_soft_step) + "\n";
    content += "#pwm_soft_step:" + String((PWM_MAX_DUTYCYCLE - PWM_MIN_DUTYCYCLE ) / g_pwm_soft_step) + "\n";
    content += "g_CoolingWaterTemp_Motor_OUT_InC: " + String(g_CoolingWaterTemp_Motor_OUT_InC) + "\n";
    content += "g_CoolingWaterTemp_Motor_IN_InC: " + String(g_CoolingWaterTemp_Motor_IN_InC) + "\n";
    content += "g_Temperature_PWM_Policy: " + String(g_Temperature_PWM_Policy) + "\n";
    content += "g_Temperature_PWM_Policy description: " + String(getPolicyDescription(g_Temperature_PWM_Policy)) + "<BR>";
    content += "C_Cooling_Policy3__MotorOut_FanONTemp: " + String(C_Cooling_Policy3__MotorOut_FanONTemp) + "\n";
    content += "C_Cooling_Policy3__MotorIN_FanONTemp: " + String(C_Cooling_Policy3__MotorIN_FanONTemp) + "\n";
    content += "C_Cooling_Policy3__MotorIN_FanOffTemp: " + String(C_Cooling_Policy3__MotorIN_FanOffTemp) + "\n";
    content += "g_Cooling_Policy3__MotorOut_FanONTemp: " + String(g_Cooling_Policy3__MotorOut_FanONTemp) + "\n";
    content += "g_Cooling_Policy3__MotorIN_FanONTemp: " + String(g_Cooling_Policy3__MotorIN_FanONTemp) + "\n";
    content += "g_Cooling_Policy3__MotorIN_FanOffTemp: " + String(g_Cooling_Policy3__MotorIN_FanOffTemp) + "\n";
    content += "C_EEPROM_SCHEMA in code: " + String(C_EEPROM_SCHEMA) + "\n";
    content += "C_EEPROM_SCHEMA on EEPROM: " + String(preferences.getUInt(Preferences_keys[C_EEPROM_SCHEMA_IDX], NO_VALUE)) + "\n";
    content += "C_PROGVERSION in code: " + String(C_PROGVERSION) + "\n";
    content += "C_PROGVERSION on EEPROM: " + String(preferences.getUInt(Preferences_keys[C_EEPROM_PROGVER_IDX], NO_VALUE)) + "\n";
    content += "g_display_update_time (ms): " + String(g_display_update_time) + "\n";
    content += "g_Dallas_Default_Ordering (true/false): " + String(g_Dallas_Default_Ordering) + "\n";

    //content += "EEPROM g_display_update_time (ms): "));
    //String(EEPROM[C_display_update_time_IDX]);
    // EEPROM[] reads just a byte, and for display update in ms which I like to have at 1000, it just doesn't fit.

    //content += "EEPROM address size: " + String(EEPROM.length()) + "\n";
    content += "g_calculated_rpm : " + String(g_calculated_rpm) + "\n";

    content += " : " + String(g_calculated_rpm) + "\n";
    content += "DallasMissedCounters[MotorTemp_IDX] : " + String(DallasMissedCounters[MotorTemp_IDX]) + "\n";
    content += "DallasMissedCounters[RadiatorTemp_IDX] : " + String(DallasMissedCounters[RadiatorTemp_IDX]) + "\n";

    preferences.end();

    /*
        Serial.print("contentlen: ");
        //int len=content.length;
        Serial.println(content.length());
        Serial.println(content.c_str());
    */
    request->send(200, "text/plain", content);
    if (g_debug) {
      Serial.print("/debug took so many bytes:");
      Serial.println(content.length());
      Serial.print("ESP.getFreeHeap: ");
      Serial.println(ESP.getFreeHeap());
    }

  });


  async_server.on("/setup", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest * request) {
    String content;
    bool api=false;   // set to true when called from machine as api

    // print the paramater via ?&aap=xx or  better post ...
    int paramsNr = request->params();
    Serial.println(paramsNr);

    for (int i = 0; i < paramsNr; i++) {
      AsyncWebParameter* p = request->getParam(i);
      Serial.print("Param name: ");
      Serial.println(p->name());
      Serial.print("Param value: ");
      Serial.println(p->value());
      Serial.println("------");
      /*
             if(strcmp("pwm_period",p->name().c_str())==0) {
                 int _period=atoi(p->value().c_str());
                 Serial.print("Setting PWM period (uS) to: ");
                 Serial.println(_period);
                 set_PWM_Period_us(_period);
             }
             if(strcmp("debug",p->name().c_str())==0) {
                 int _period=atoi(p->value().c_str());
                 Serial.print("Setting PWM period (uS) to: ");
                 Serial.println(_period);
                 set_PWM_Period_us(_period);
             }
      */
    }

    // https://github.com/me-no-dev/ESPAsyncWebServer#handlers-and-how-do-they-work
    //Check if GET parameter exists
    // type=Checkbox param isn;t included when default (off)
    if (request->hasParam("debug", true)) {
    //      AsyncWebParameter* p = request->getParam("debug", true);
    //  if ( strncmp(p->value().c_str(),"on", strlen("on")) == 0) {
        // on
        if (g_debug) Serial.println("Setting g_debug to: on");
        g_debug = true;
      } else {
        // off
        if (g_debug) Serial.println("Setting g_debug to: off");
        g_debug = false;
      }
    //}


    // type= tekst
    if (request->hasParam("coolingpolicy", true)) {
      AsyncWebParameter* p = request->getParam("coolingpolicy", true);
      int _cp = p->value().length() == 0  ? NO_VALUE  : atoi(p->value().c_str());
      if (g_debug) {
        Serial.print("Setting g_Temperature_PWM_Policy to: ");
        Serial.println(_cp);
      }
      if (_cp >= 0 ) g_Temperature_PWM_Policy = _cp;
    }


    // type= tekst
    if (request->hasParam("operatingmode", true)) {
      AsyncWebParameter* p = request->getParam("operatingmode", true);
      int _cp = p->value().length() == 0  ? NO_VALUE  : atoi(p->value().c_str());
      if (g_debug) {
        Serial.print("Setting g_operating_mode to : ");
        Serial.println(_cp);
      }
      if (_cp >= 0 ) g_operating_mode = _cp;
    }


    if (request->hasParam("pwm_period", true)) {
      AsyncWebParameter* p = request->getParam("pwm_period", true);
      int _cp = p->value().length() == 0  ? NO_VALUE  : atoi(p->value().c_str());
      if (g_debug) {
        Serial.print("Setting PWM_Period_us to: ");
        Serial.println(_cp);
      }
      if ( _cp > 0 ) {
        if (g_operating_mode != C_OPERATING_MODE_MANUAL) set_PWM_Period_us(_cp);
        g_PWM_Period = _cp;   // store setting for future re-use
      }
    }

    if (request->hasParam("manual_PWM_dutycycle", true)) {
      AsyncWebParameter* p = request->getParam("manual_PWM_dutycycle", true);
      int _cp = p->value().length() == 0  ? NO_VALUE  : atoi(p->value().c_str());
      if (g_debug) {
        Serial.print("Setting g_completely_manual_motor_control_pwm_dutycycle to: ");
        Serial.println(_cp);
      }
      if ( _cp > 0 ) g_completely_manual_motor_control_pwm_dutycycle = _cp;   // value is picked up in eventloop: update_PWM_dutycycle_desired();
    }

    if (request->hasParam("manual_PWM_period", true)) {
      AsyncWebParameter* p = request->getParam("manual_PWM_period", true);
      int _cp = p->value().length() == 0  ? NO_VALUE  : atoi(p->value().c_str());
      if (g_debug) {
        Serial.print("Setting g_completely_manual_motor_control_pwm_period_us to: ");
        Serial.println(_cp);
      }
      if ( _cp > 0 ) {
        if (g_operating_mode == C_OPERATING_MODE_MANUAL) set_PWM_Period_us(_cp);
        g_completely_manual_motor_control_pwm_period_us = _cp;
      }
    }

    if (request->hasParam("fansoftstarttime", true)) {
      AsyncWebParameter* p = request->getParam("fansoftstarttime", true);
      int _cp = p->value().length() == 0  ? NO_VALUE  : atoi(p->value().c_str());
      if (g_debug) {
        Serial.print("g_FAN_SLOWSTART_TIME: ");
        Serial.println(_cp);
      }
      //g_FAN_SLOWSTART_TIME=_cp;
      if ( _cp > 0 ) set_pwm_control_timer(_cp);
    }

    if (request->hasParam("simulated_motortemp", true)) {
      AsyncWebParameter* p = request->getParam("simulated_motortemp", true);
      float _cp = p->value().length() == 0  ? NO_VALUE  : p->value().toFloat();
      if (g_debug) {
        Serial.print("g_simulate_values[MotorTemp_IDX]: ");
        Serial.println(_cp);
      }
      //g_FAN_SLOWSTART_TIME=_cp;
      if ( _cp > 0 ) g_simulate_values[MotorTemp_IDX] = _cp;
    }
    if (request->hasParam("simulated_radiatortemp", true)) {
      AsyncWebParameter* p = request->getParam("simulated_radiatortemp", true);
      float _cp = p->value().length() == 0  ? NO_VALUE  : p->value().toFloat();
      if (g_debug) {
        Serial.print("g_simulate_values[RadiatorTemp_IDX]: ");
        Serial.println(_cp);
      }
      //g_FAN_SLOWSTART_TIME=_cp;
      if ( _cp > 0 ) g_simulate_values[RadiatorTemp_IDX] = _cp;
    }

    if (request->hasParam("nvram", true)) {
      AsyncWebParameter* p = request->getParam("nvram", true);
      int _cp = p->value().length() == 0  ? NO_VALUE  : atoi(p->value().c_str());
      if (g_debug) {
        Serial.print("nvram: ");
        Serial.println(p->value().c_str());
      }
      if ( _cp == 1  ) save_running_config();
    }
    if (request->hasParam("Display_Refresh_time_ms", true)) {
      AsyncWebParameter* p = request->getParam("Display_Refresh_time_ms", true);
      int _cp = p->value().length() == 0  ? NO_VALUE  : atoi(p->value().c_str());
      if (g_debug) {
        Serial.print("g_display_update_time: ");
        Serial.println(_cp);
      }
      //g_FAN_SLOWSTART_TIME=_cp;
      if ( _cp > NO_VALUE ) g_display_update_time = _cp;
    }
    if (request->hasParam("Default_Order_Dallas", true)) {
      AsyncWebParameter* p = request->getParam("Default_Order_Dallas", true);
      int _cp = p->value().length() == 0  ? NO_VALUE  : atoi(p->value().c_str());
      if (g_debug) {
        Serial.print("Default_Order_Dallas: ");
        Serial.println(_cp);
      }
      //g_FAN_SLOWSTART_TIME=_cp;
      if ( _cp > NO_VALUE ) g_Dallas_Default_Ordering = _cp;
    }

    if (request->hasParam("api", true)) {
      AsyncWebParameter* p = request->getParam("api", true);
      int _cp = p->value().length() == 0  ? NO_VALUE  : atoi(p->value().c_str());
      if (g_debug) {
        Serial.print("api: ");
        Serial.println(_cp);
      }
      //g_FAN_SLOWSTART_TIME=_cp;
      if ( _cp > NO_VALUE ) api = _cp;
    }

  if(api>0) {
      content = "OK";
  } else { 


    content = "<html><head></head><body>";

    content += "<h1>Welcome to uP fan Control setup</h1>";

    content += "<form method=post>";

    content += "<fieldset> <legend>Cooling Policy function</legend>";

      for (int p=0 ; p < NR_CFunctions(); p++) {
        content += "<input type='radio' name=coolingpolicy value='" + String(p) +  "'";
         if (g_Temperature_PWM_Policy == p) content += "checked";
         content += "> " + String(getPolicyDescription(p)) + "<br>";     
      }

    content += "</fieldset>";


    content += "<fieldset> <legend>Operating mode</legend>";

    content += "<input type='radio' name=operatingmode value='";
    content += C_OPERATING_MODE_AUTOMATIC;
    content += "'";
    if (g_operating_mode == C_OPERATING_MODE_AUTOMATIC) content += "checked";
    content += ">Automatic<br>";
    content += "<input type='radio' name=operatingmode value='";
    content += C_OPERATING_MODE_SIMULATED;
    content += "'";
    if (g_operating_mode == C_OPERATING_MODE_SIMULATED) content += "checked";
    content += ">Simulated temperature values<br>";
    content += "<input type='radio' name=operatingmode value='";
    content += C_OPERATING_MODE_MANUAL;
    content += "'";
    if (g_operating_mode == C_OPERATING_MODE_MANUAL) content += "checked";
    content += "> Manual Controll<br>";

    content += "</fieldset>";




    content += "<label class=\"switch\"> <input type=\"checkbox\" name=debug value=on ";
    if (g_debug) content += "checked";
    content += "> Debug<BR><span class=\"checkbox round\"></span></label> ";

    content += "<fieldset> <legend>Automatic mode: Fan Motor options</legend>";

    content += "<input type=\"text\" name=fansoftstarttime placeholder=\"";
    content += g_FAN_SLOWSTART_TIME;    // yes arduino IDE can do this!!
    content += "\">FanMotor Softstart time<BR>";
    // Used to test PWM and motor

    content += "<input type=\"text\" name=pwm_period placeholder=\"";
    content += g_PWM_Period;    // yes arduino can do this.
    content += "\">PWM_period<BR>";

    content += "</fieldset>";


    content += "<fieldset> <legend>Manual Control mode: Fan Motor options</legend>";

    content += "<input type=\"text\" name=manual_PWM_dutycycle placeholder=\"";
    content += g_completely_manual_motor_control_pwm_dutycycle;   // yes arduino can do this
    content += "\"> Manual PWM dutycycle<BR>";

    content += "<input type=\"text\" name=manual_PWM_period placeholder=\"";
    content += g_completely_manual_motor_control_pwm_period_us;
    content += "\"> Manual PWM Period<BR>";

    content += "</fieldset>";



    content += "<fieldset> <legend>Simulation mode: temperature options</legend>";

    content += "<input type=\"text\" name=simulated_motortemp placeholder=\"";
    content += g_simulate_values[MotorTemp_IDX];   // yes arduino can do this
    content += "\"> Simulated motortemp<BR>";

    content += "<input type=\"text\" name=simulated_radiatortemp placeholder=\"";
    content += g_simulate_values[RadiatorTemp_IDX];
    content += "\"> Simulated radiatortemp<BR>";

    content += "</fieldset>";

    content += "<fieldset> <legend>Various Settings</legend>";

    content += "<input type=\"text\" name=Display_Refresh_time_ms placeholder=\"";
    content +=  g_display_update_time;
    content += "\">Display Refresh time (ms)<BR>";

    content += "<input type=\"text\" name=Default_Order_Dallas placeholder=\"";
    content +=  g_Dallas_Default_Ordering;
    content += "\">Default Order Dallas Sensors (true=default, false=reverse)<BR>";

    content += "</fieldset>";



    content += "<fieldset> <legend>WiFi Client/Server options</legend>";

    content += "<input type=\"text\" name=WiFI_Server_SSID>WiFI Server SSID<BR>";
    content += "<input type=\"text\" name=WiFI_Server_passwd>WiFI Server password<BR>";
    content += "<input type=\"text\" name=WiFI_Client_SSID>WiFI Client SSID<BR>";
    content += "<input type=\"text\" name=WiFI_Client_password>WiFI Client password<BR>";

    content += "</fieldset>";




    content += "<input type=submit value=submit></form>";

    content += "<form>";
    content += "<input name=\"nvram\" type=\"hidden\" value=\"true\">";
    content += "<input type=submit value=\"Store into NVRAM/EEPROM\"></form>";

    content += "<a href=\"/\">Home</a><BR>";

    content += "</body></html>";
    //content += "g_debug: " + String(g_debug) + "\n";

  }
    request->send(200, "text/html", content);

    if (g_debug) {
      Serial.print("/setup took so many bytes:");
      Serial.println(content.length());
      Serial.print("ESP.getFreeHeap: ");
      Serial.println(ESP.getFreeHeap());
    }
  });



  async_server.on("/hello", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "Hello World");
  });

// contains some commented unknown members/call's!!!
// Simple Firmware Update Form
  async_server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });
  
  async_server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      Serial.printf("Update Start: %s\n", filename.c_str());
      // Update.runAsync(true);   TOCHECK!!!! See https://github.com/me-no-dev/ESPAsyncWebServer
      //Update::runAsynchronously();
      //if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){      // TOFIX!!!
      //uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      //if(!Update.begin(0x140000)) {
      if(!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        Serial.printf("Update Success: %uB\n", index+len);
      } else {
        Update.printError(Serial);
      }
    }
  });



  //sync_server.begin();
  async_server.begin();
  Serial.println("Started async webserver");


  //Events part


  events.onConnect([](AsyncEventSourceClient * client) {
    Serial.printf("--Client reconnected! Last message ID that it gat is\n");

    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
    }
    //send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), SSE_reconnect_time_ms);
    g_Enforce_HTTP5_SSE_update = true;  // enforce all parameters to all clients next time once., as it is a personal device, this will do it for now.

  });

  //HTTP Basic authentication
  //events.setAuthentication("user", "pass");
  async_server.addHandler(&events);

  Serial.println("Registered SSE /events in webserver");


}








/*
  void checkWifiClient()
  {
  WiFiClient client = sync_server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    //delay(1); // prevent hanging arduino for ever???
    uint32_t ultimeout = millis() + 250;
    while (client.connected() && (millis() < ultimeout)) {            // loop while the client's connected
      //delay(1); // prevent hanging arduino for ever???
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(5, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(5, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
  }
*/


// refreshes values, but throttled refresh speed
// when not throttled, the PWM value updates may (will) lock up the ESP32 ... (when no near perfect connection)
void update_html5_sse_values_throttled(bool enforced) {

  static uint32_t  previous_time=millis();
  uint32_t current_time;
  current_time=millis();

  if( ( (current_time - previous_time) > C_SSE_Update_throttle_time_ms ) || (enforced)) {
    update_html5_sse_values_now(enforced);
    previous_time=current_time;
  }
}

// update the values's only when an update is needed.
// recommended to check often in event loop, but be aware of high frequency updates which might get too fast (and crash the esp32)

void update_html5_sse_values_now(bool enforced) {
  static float previous_g_CoolingWaterTemp_Motor_IN_InC = NO_VALUE;
  static float previous_g_CoolingWaterTemp_Motor_OUT_InC = NO_VALUE;
  static long int previous__dutycycle_actual = NO_VALUE;
  static long int previous_dutycycle_desired = NO_VALUE;
  static uint32_t  previous_seconds = NO_VALUE;
  static int previous_g_Temperature_PWM_Policy = NO_VALUE;
  static int previous_g_operating_mode = NO_VALUE;

  char buffer[C_STR_BUFSIZE];


  if (g_Enforce_HTTP5_SSE_update) {
    enforced = true; // enforce updates after a (re)connect.
    g_Enforce_HTTP5_SSE_update = false; // only once!!!!!
    Serial.println("g_Enforce_HTTP5_SSE_update !!!! updating all paramters to all clients");
    //delay(1000);    // this seems to get faste then the registrtion??
  }


  if ( (previous_g_CoolingWaterTemp_Motor_IN_InC != g_CoolingWaterTemp_Motor_IN_InC) || enforced) {
    snprintf(buffer, C_STR_BUFSIZE, "%3.2f", g_CoolingWaterTemp_Motor_IN_InC);
    if(g_debug>0) Serial.println("DEBUG-SSE: Before events.sent radatorvalue");
    events.send(buffer, "radiatorvalue", millis(), SSE_reconnect_time_ms);

    previous_g_CoolingWaterTemp_Motor_IN_InC = g_CoolingWaterTemp_Motor_IN_InC; // prevent unneeded screen updates in future.
  }

  if ( (previous_g_CoolingWaterTemp_Motor_OUT_InC != g_CoolingWaterTemp_Motor_OUT_InC ) || enforced) {
    snprintf(buffer, C_STR_BUFSIZE, "%3.2f", g_CoolingWaterTemp_Motor_OUT_InC);
    if(g_debug>0) Serial.println("DEBUG-SSE: Before events.sent motorvalue");
    events.send(buffer, "motorvalue", millis(), SSE_reconnect_time_ms);  // Event

    previous_g_CoolingWaterTemp_Motor_OUT_InC = g_CoolingWaterTemp_Motor_OUT_InC; // prevent unneeded screen updates in future.
  }


  // TODO inject error message when PWM value isn't checked/updated.
  if ( (previous__dutycycle_actual != __dutycycle_actual) || enforced) {
    snprintf(buffer, C_STR_BUFSIZE, "%4ld", __dutycycle_actual);
    if(g_debug>0) Serial.println("DEBUG-SSE: Before events.sent dutycycle actual");
    events.send(buffer, "dutycycle_actual", millis(), SSE_reconnect_time_ms);  // Event

    previous__dutycycle_actual = __dutycycle_actual;  // prevent unneeded screen updates in future.
  }

  // TODO, inject error message when Cooling Policy isn't updated
  static bool previous_Check_runs_Cooling_Policy=true;
  bool current_Check_runs_Cooling_Policy=Check_runs_Cooling_Policy();
  
  if ( (previous_dutycycle_desired != dutycycle_desired) || enforced || (current_Check_runs_Cooling_Policy == true  && previous_Check_runs_Cooling_Policy == false) ) {
    snprintf(buffer, C_STR_BUFSIZE, "%ld/<BR>\n%d", dutycycle_desired, PWM_MAX_DUTYCYCLE);
    if(g_debug>0) Serial.println("DEBUG-SSE: Before events.sent pwmunit (1)");
    events.send(buffer, "pwmunit", millis(), SSE_reconnect_time_ms);  // Event
    previous_dutycycle_desired = dutycycle_desired; // prevent unneeded screen updates in future.
  } else {
    // if not updated, check if the mechanism has run to actually change the variabele dutycycle_desired
    if (current_Check_runs_Cooling_Policy == false  && previous_Check_runs_Cooling_Policy == true) {    // only once
      // replace value with error message
      int ret=0;
      ret=snprintf(buffer, C_STR_BUFSIZE, "<font color=\"red\">%ld</font>/<BR>\n%d", dutycycle_desired, PWM_MAX_DUTYCYCLE);
      // error handling, as this may not get ignored 
      if(ret > C_STR_BUFSIZE) {
        Serial.print("snprintf buffer too small __FILE__ LINE__  ");
        Serial.println(ret);
      }
      if(g_debug>0) Serial.println("DEBUG-SSE: Before events.sent pwmunit (2)");    
      events.send(buffer, "pwmunit", millis(), SSE_reconnect_time_ms);  // Event
        
    }
  }
  previous_Check_runs_Cooling_Policy=current_Check_runs_Cooling_Policy;




  if ( (previous_seconds != millis() / 1000) || enforced)  {
    get_RunningTime(buffer, C_STR_BUFSIZE);
    if(g_debug>0) Serial.println("DEBUG-SSE: Before events.sent uptime");    
    events.send(buffer, "uptime", millis(), SSE_reconnect_time_ms);  // Event

    previous_seconds = millis() / 1000; // prevent unneeded screen updates in future.
  }

  if ( (previous_g_Temperature_PWM_Policy != g_Temperature_PWM_Policy) || enforced) {
    snprintf(buffer, C_STR_BUFSIZE, "%d", g_Temperature_PWM_Policy);
    //events.send(buffer, "policy", millis());  // Event
    if(g_debug>0) Serial.println("DEBUG-SSE: Before events.sent policy");    
    events.send(getPolicyDescription(g_Temperature_PWM_Policy), "policy", millis(), SSE_reconnect_time_ms);  // Event 
    
    previous_g_Temperature_PWM_Policy = g_Temperature_PWM_Policy; // prevent unneeded screen updates in future.
  }

  if ( (previous_g_operating_mode != g_operating_mode) || enforced) {
    if(g_debug>0) Serial.println("DEBUG-SSE: Before events.sent operating mode");    
    if ( g_operating_mode == C_OPERATING_MODE_AUTOMATIC) events.send("AUTOMATIC", "control", millis(), SSE_reconnect_time_ms);
    if ( g_operating_mode == C_OPERATING_MODE_SIMULATED) events.send("SIMULATED", "control", millis(), SSE_reconnect_time_ms);
    if ( g_operating_mode == C_OPERATING_MODE_MANUAL)    events.send("MANUAL", "control", millis() , SSE_reconnect_time_ms);

    previous_g_operating_mode = g_operating_mode; // prevent unneeded screen updates in future.
  }

}




#endif // USE_HTTP_SERVER

