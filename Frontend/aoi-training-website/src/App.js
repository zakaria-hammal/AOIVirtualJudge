import { useState } from "react";
import "./App.css";

function App() {
  const [name, setName] = useState("");
  const [email, setEmail] = useState("");
  const [file, setFile] = useState(null);
  const [status, setStatus] = useState("");

  const handleSubmit = async (e) => {
    e.preventDefault();

    const formData = new FormData();
    formData.append("name", name);
    formData.append("email", email);
    formData.append("solution", file);

    try {
      const response = await fetch("http://localhost:3000/submit", {
        method: "POST",
        body: formData,
      });

      if (response.ok) {
        const data = await response.json();
        setStatus(data.message);
      } else {
        setStatus("❌ Submission failed.");
      }
    } catch (err) {
      console.error(err);
      setStatus("⚠️ Error connecting to server.");
    }
  };

  return (
    <div className="App">
      <header className="App-header">
        <form onSubmit={handleSubmit} className="space-y-4">
          <div>
            <label className="block">Name:</label>
            <input
              type="text"
              value={name}
              onChange={(e) => setName(e.target.value)}
              className="border p-2 rounded w-full"
              required
            />
          </div>

          <div>
            <label className="block">Email:</label>
            <input
              type="email"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              className="border p-2 rounded w-full"
              required
            />
          </div>

          <div>
            <label className="block">Your Solution :</label>
            <input
              type="file"
              onChange={(e) => setFile(e.target.files[0])}
              className="border p-2 rounded w-full"
              required
            />
          </div>

          <button
            type="submit"
            className="bg-blue-600 text-white px-4 py-2 rounded"
          >
            Submit
          </button>
        </form>

        {status && <p className="mt-4">{status}</p>}
      </header>
    </div>
  );
}

export default App;
