const express = require("express");
const app = express();
app.use(express.json());
app.use(express.urlencoded({ extended: true }));


let response = 0;
const arr = {};
app.get("/log", (req, res) => {
    res.json(arr);
})
app.post("/data", (req, res) => {
    const ip = req.socket.remoteAddress;
    console.log(ip);

    res.json({ message: "mukhiya is a good boy" });
})
app.listen(4000, () => {
    console.log("Running");
});