# Hosting a React.JS web application from an ESP 32 using Async server, webpack, and more

# Introduction

The ESP32 is a powerful little piece of hardware that has a wide array of uses and possibilities. One major feature of the ESP32 is its ability to add wireless connectivity to almost any electronics project.

React is another powerful tool. React is the industry standard for developing front end web applications. Using JavaScript, React provides a framework for developing reactive UIs.

Today I will show you how to merge these two technologies, unlocking the ability to create large, fully featured web applications connected with your hardware projects. Together, we will create a web application that controls the color of the built in LED on the ESP32-S2.

Note: The S2 version is not required to host a react application, it's just the one I am using and you may need to modify the IO pins to follow along with your board.

# What you need

- An ESP32. I am using an ESP32-S2-DevKit available from Mouser for $8.
- Arduino IDE
- VSCode with optional Arduino extension
- [ESP32 FS uploader](https://github.com/me-no-dev/arduino-esp32fs-plugin)
  - If using the S2, use [this fork](https://github.com/etherfi/arduino-esp32fs-plugin-esp32s2)
- External Libraries:
  - [ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
  - [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
  - [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- Arduino Libraries:
  - Adafruit NeoPixel
- NodeJS
- Yarn

For the sake of concision, I will not be covering the installation of these tools.

# Step 1 - React in a Pack

Setting up the React application is, in my opinion, the most difficult piece and the key to this project. As React is simply a javascript framework there are many different ways to set up a project, and many of the most popular incarnations from the web development community are not appropriate for our use case.

I will now ramble on about why we are doing this, if you don't care feel free to skip this section. In the web development world that React comes from, web applications are usually served and hosted on servers. These servers are typically in large datacenters and have access to unprecedented amounts of compute power. In this case, the device the user is connecting from is the weakest link, so much of react tooling is built with this perspective. Our perspective, however, is severely different. The device you are connecting from is incomprehensibly faster than the ESP32, and we need to make sure this is considered.

Our desired outcome is to ensure that the ESP32 is doing the minimum possible work and we offload as much as possible to the "user's" device. We can configure the ESP32 to only serve the files and all the UI processing will happen in the user's browser. In addition, it is generally easier to work with data in Javascript as we don't have the constraints of C++ and embedded systems.

### I. React Boiler

We will bootstrap react with.... nothing! That's right we are going to create a custom react app and then bundle it together with webpack. This will allow us to use all the features of React and shove it all into a couple nice little files that we can serve from the ESP32.

1.  Create a new folder. This will contain the whole project.
2.  Inside the new folder, create another new folder, called app. This will contain the React portion.
3.  cd into app/. Run the command `yarn init -y`
4.  `yarn add react react-dom`
5.  Create two more directories, `src`, `public`.
    1.  In `src` create 2 files: `index.js`, `App.js`
    2.  In public create 2 files: `index.html` and an icon `favicon.ico`
6.  Open the app folder in VSCode with `code .`
7.  Lets now populate these files with some basic React boilerplate:

index.js:

```javascript
import React from "react";
import ReactDOM from "react-dom";

import App from "./App";

ReactDOM.render(<App />, document.getElementById("root"));
```

App.js

```javascript
import React from "react";

export default function App() {
  return (
    <div className="App">
      <h1>Hello World</h1>
    </div>
  );
}
```

index.html

```html
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta
      name="viewport"
      content="width=device-width, initial-scale=1, shrink-to-fit=no"
    />
    <meta name="theme-color" content="#000000" />

    <link rel="shortcut icon" href="%PUBLIC_URL%/favicon.ico" />

    <title>ESP32 React Server</title>
  </head>

  <body>
    <noscript> You need to enable JavaScript to run this app. </noscript>
    <div id="root"></div>
  </body>
</html>
```

### II. Pack Your Bags

Now we have a react application, congrats! But it's pretty useless, we can't even run it to see what it looks like. Lame! It's okay, after this headache we should have a working dev environment.

For this, you're gonna wanna just copy my shit, webpack is kind of convoluted. If you're interested read the documentation.

Install the dependencies with `yarn`
Run the dev server with `yarn start`

### III. Construction Commences

1.  With webpack working we can install any packages we want! Lets install a color picker with `yarn add @uiw/react-color-sketch`
2.  And we can add the new component to the react app.

```javascript
<div className="colorPicker">
  <Sketch
    color={hex}
    onChange={(color) => {
      setHex(color.hex);
    }}
  />
</div>
```

4.  Build the app like normal. We can add styles with `import "./styles.css";` and even create separate component or utility files. All of this will get bundled at the end of the day.

### IV. Talking With Others

It would be great if the web server could not only be hosted by the ESP32, but could also communicate and send commands!
Using the Fetch API, this is fairly straightforward.

```javascript
export function postLed(hex) {
  fetch(`/api/led`, {
    method: "POST",
    body: parseInt(hex.substr(1), 16),
  });
}
```

Here, we keep the POST request as simple as possible (KISS) because processing the body on the ESP32 is not as easy.

1.  `/api/led` This will be the api path to implement on the ESP32.
2.  `method: "POST"` Since we are sending data we will use a POST request (see HTTP headers for more info).
3.  `parseInt(hex.substr(1), 16)` Just some fancy JS to clean up the hex value so that we have as little processing to do on the ESP32 as possible. (Remember the device the front-end is running on is lightyears quicker than the ESP32 is.)

### V. Build the Bundle

1.  Run `yarn build`. Hopefully no errors pop up! 🤞
2.  In the `app/dist` folder you should see a couple files, these are the important ones:
    1.  `favicon.ico`
    2.  `index.html`
    3.  `index.js`

That's it! That's the whole React app! You can actually open `index.html` in your browser and the app should work just like it did in the development environment. Now, on to the microcontroller!

# Step 2 - Size Doesn't Matter

### I. Data Makes the World Go Round

To set up the arduino project, lets create another folder in the project folder.

1.  In the project folder, create the folder `appserver`,
2.  In this new folder create a file `appserver.ino`
3.  Also in this new folder, create a folder `data/`.

A note on naming: **The folder MUST BE NAMED DATA !!** The folder and the `.ino` file _must_ have the same names (but it can be whatever your heart desires 💗).

4.  Copy the important files from the `app/dist/` folder into the `appserver/data/` folder.
5.  In the Arduino IDE, open the `appserver/` folder.
6.  Connect your ESP32. Select _Tools_ \> _Esp32 Sketch Data Uploader_, and wait for the data to upload.

The ESP32 Sketch Data Uploader or ESP32 FS Uploader, uploads all files in the sketch's data folder into the device's SPIFFS filesystem. Please note that sub-directories (e.x. `/data/newfolder/myfile.txt`) are **NOT** supported (unless you are using LittleFS, which we, right now, unfortunately, are not using). If you have any questions please refer to the ESP32 SPIFFS and FS documentation (and let me know if you find any useful information because I found none).

### Painters Paint and Coders... Sketch?

Finally! We get to write code for the ESP32! All this time and not a single crumb of C++. Doesn't it feel good? The wind in your hair, the sun beaming down, and a nice cup of javascript to wash it all down. Well, you better put that cup of joe down, because it's time to get _serious_ (get it.. like C-erious... like C but it's.. you know what, nevermind).

This is another portion where it might be easier just to look at the full code, but I'll walk through some important parts and sections I found difficult.

**_WIFIY_**

```c++
const char *ssid = "YourSSID";
const char *password = "Your Password";
const bool apMode = false;

...
void setup()
{
    ...

// ### Start WiFi
    if (apMode)
    {
        ...
    }
}
```

Please enter your network SSID and password up at the top if you want the web server to be available on your network. Otherwise, you can set `apMode = true` and the ESP32 will create its own wireless network and host the server there.

**_SPIFFY_**

```c++
// ### Start SPIFFS
if (!SPIFFS.begin(true))
{
    Serial.println("SPIFFS Mount Failed");
    return;
}
```

This is how I start SPIFFS, and it works for me. 🤷

**_Asyncy_**

```c++
AsyncWebServer *server;

void setup()
{
    // ### Start Server
    server = new AsyncWebServer(80);
    ...
```

Mind the Async server boilerplate. (I did it a little weird because I pulled this code from custom library created for another project)

```c++
// ### Server Routes
server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
           { request->send(SPIFFS, "/index.html", "text/html"); });

server->on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request)
           { request->send(SPIFFS, "/index.js", "text/javascript"); });

server->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
           { request->send(SPIFFS, "/favicon.ico", "image/x-icon"); });

```

Lets talk about these server routes. These three routes serve the files necessary for the React application to run. Only the smartest of readers will have noticed that these routes conspicuously correspond to the list of important files we purposely placed into the data folder.

```c++
// ### API
server->on(
    "/api/led", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
    {
        char *str = (char *)data;
        Serial.println(atoi(str));
        pixel.setPixelColor(0, pixel.gamma32(atoi(str)));
        pixel.show();

        request->send(200);
    });
```

Now, for the API route. This beast of boilerplate is super simple once the crap is removed.

```c++
char *str = (char *)data;
pixel.setPixelColor(0, pixel.gamma32(atoi(str)));
pixel.show();

request->send(200);
```

Here, extracted from the distractions, is the code that converts the message sent from the server into changing the pixel color. The `data` variable points to an array of `uint8_t` values, i.e. the list of chars from the request body. In our case, since we processed everything out in javascript and sent the data over raw and ready to go, we simply cast a few times to convert the char array into the integer values the pixel library wants.

For the more complex, JSON processing is possible using a library called [ArduinoJson](https://arduinojson.org/), but that is a whole beast in itself.

```c++
...
    server->begin();
}
```

Last but not least, don't forget to start the server.

```c++
void loop()
{
}
```

And a blank Loop is required to get Arduino to compile. Nothing needs to happen in the loop since the server runs out of sync (asynchronously).

# Step 3 - It's Showtime

That's (hopefully) it! Build and upload the sketch, get the IP address from the Serial monitor, and type that bad boy into your browser of choice! You should (hopefully) now be looking at a fully functional React app hosted on a toaster, woops, I mean ESP32.
