import React from "react";
import ReactDOM from "react-dom";

import App from "./App";
import "./styles.css";

// Webpack hot module replacement
if (module.hot) module.hot.accept();

ReactDOM.render(<App />, document.getElementById("root"));
