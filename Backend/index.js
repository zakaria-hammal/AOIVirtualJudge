const express = require("express");
const cors = require("cors");
const multer = require("multer");
const path = require("path");
const fs = require("fs");
const { spawn } = require("child_process");

const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, "Solution/");
  },
  filename: (req, file, cb) => {
    cb(null, "solution.cpp");
  },
});

const upload = multer({ storage });
const app = express();

app.use(cors());

// Route for form submission
app.post("/submit", upload.single("solution"), (req, res) => {
  console.log("Fields:", req.body);
  console.log("File saved as:", req.file.path);

  const child = spawn("sudo", ["./judge", "1", "0", "0"], {
    stdio: "inherit",
  });

  child.on("close", (code) => {
    console.log(`Judge exited with code ${code}`);

    try {
      const data = fs.readFileSync("judgeOutput.out", "utf8");
      console.log("the data :", data);
      res.send({ message: data });
    } catch (err) {
      console.error("Error reading judge output:", err);
      res.status(500).send({ message: "Error reading judge output" });
    }
  });
});

app.listen(3000, () => {
  console.log("Server running at http://localhost:3000");
});
