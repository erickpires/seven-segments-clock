#ifndef SERVER_PAGE_HTML_H
#define SERVER_PAGE_HTML_H

#define CONFIG_PAGE_HTML(CONFIG) \
"<!DOCTYPE HTML>" \
"<html>" \
  "<body>" \
    "<h1>7-segments clock configuration:</h1>" \
    "<form method='get' action='settings'>" \
      "<label>Timezone hour offset: </label>" \
      "<input name='tz-hour-offset' type='number' min='-12' max='12' value='" + String(CONFIG.timezoneHoursOffset) + "'>" \
      "<br/>" \
      "<label>Timezone minutes offset: </label>" \
      "<input name='tz-minutes-offset' type='number' min='0' max='59' value='" + String(CONFIG.timezoneMinutesOffset) + "'>" \
      "<br/>" \
      "<label>Daylight saving: </label>" \
      "<input name='daylight-saving' type='checkbox'" + (CONFIG.isInDaylightSaving ? "checked" : "") + ">" \
      "<br/>" \
      "<label>Display brightness: </label>" \
      "<input name='brightness' type='range' min='10' max='255' value='" + String(CONFIG.displayBrightness) + "'>" \
      "<br/>" \
      "<label>Decrease brightness during night: </label>" \
      "<input name='decrease-brightness' type='checkbox'" + (CONFIG.decreaseBrightnessDuringNight ? "checked" : "") + ">" \
      "<br/>" \
      "<input type='submit' value='Save settings'/>" \
    "</form>" \
  "</body>" \
"</html>"

#define SAVED_PAGE_HTML \
"<!DOCTYPE HTML>" \
"<html>" \
  "<body>" \
    "<h1>Saved!</h1>" \
    "<button onclick='history.back()'>Go back</button>" \
  "</body>" \
"</html>"

#endif