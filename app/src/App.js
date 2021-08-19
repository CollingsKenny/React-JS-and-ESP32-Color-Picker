import React, { useEffect, useState } from "react";
import { postLed } from "./util";
import Sketch from "@uiw/react-color-sketch";

export default function App() {
  const [hex, setHex] = useState("#fff");
  useEffect(async () => {
    postLed(hex);
  }, [hex]);
  return (
    <div className="App" style={{ backgroundColor: hex }}>
      <div className="container">
        <h1>ESP 32 Color Picker</h1>
        <div className="colorPicker">
          <Sketch
            color={hex}
            onChange={(color) => {
              setHex(color.hex);
            }}
          />
        </div>
      </div>
    </div>
  );
}
