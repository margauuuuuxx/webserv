#!/usr/bin/python3
import random
import datetime

# --- Dynamic random elements ---
messages = [
    "Whoa, you're inside a Python CGI script! 🐍",
    "This page was generated on the fly! ✨",
    "Your Webserv is alive and kicking 🚀",
    "Dynamic, interactive, and playful CGI! 🎨",
    "Look at this spinning shape! 🔥",
    "Refresh for a brand-new surprise 🎲"
]

shapes = ["circle", "square", "triangle"]
colors = ["#007bff", "#28a745", "#ffc107", "#dc3545", "#17a2b8", "#ff69b4"]

# Random picks
message = random.choice(messages)
shape = random.choice(shapes)
shape_color = random.choice(colors)
button_color = random.choice(colors)
emoji = random.choice(["😎", "🎉", "🔥", "🐍", "💻", "✨", "🌟", "🎲"])

# Current time
current_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# --- Generate shape CSS dynamically ---
if shape == "circle":
    shape_css = f"border-radius: 50%; background: {shape_color};"
elif shape == "square":
    shape_css = f"background: {shape_color};"
elif shape == "triangle":
    shape_css = (
        "width: 0; height: 0; "
        "border-left: 50px solid transparent; "
        "border-right: 50px solid transparent; "
        f"border-bottom: 100px solid {shape_color};"
    )

# --- Output HTML ---
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Webserv CGI Playground</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }}
        body {{
            background: linear-gradient(135deg, #f5f7fa, #c3cfe2);
            color: #333;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            text-align: center;
            overflow: hidden;
        }}
        .container {{
            background: rgba(255, 255, 255, 0.9);
            padding: 40px 60px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1);
            max-width: 650px;
            position: relative;
            z-index: 2;
        }}
        h1 {{
            font-size: 3rem;
            margin-bottom: 15px;
            color: #007bff;
        }}
        p {{
            font-size: 1.2rem;
            color: #555;
            margin-bottom: 20px;
        }}
        .time {{
            font-size: 1rem;
            color: #777;
            margin-bottom: 20px;
        }}
        .shape {{
            width: 100px;
            height: 100px;
            margin: 30px auto;
            {shape_css}
            animation: spin 4s linear infinite;
        }}
        @keyframes spin {{
            0% {{ transform: rotate(0deg); }}
            100% {{ transform: rotate(360deg); }}
        }}
        .buttons {{
            display: flex;
            justify-content: center;
            gap: 20px;
        }}
        .btn {{
            padding: 15px 30px;
            font-size: 1.2rem;
            color: #fff;
            background-color: {button_color};
            border: none;
            border-radius: 5px;
            text-decoration: none;
            transition: background-color 0.3s ease, transform 0.2s ease;
        }}
        .btn:hover {{
            transform: scale(1.05);
            filter: brightness(0.9);
        }}
        .emoji {{
            font-size: 2.5rem;
            margin-bottom: 10px;
        }}
        /* Floating background bubbles for extra fun */
        .bubble {{
            position: absolute;
            border-radius: 50%;
            opacity: 0.6;
            animation: float 12s ease-in-out infinite;
        }}
        @keyframes float {{
            0% {{ transform: translateY(0) translateX(0); }}
            50% {{ transform: translateY(-50px) translateX(20px); }}
            100% {{ transform: translateY(0) translateX(0); }}
        }}
    </style>
</head>
<body>
    <!-- Random floating bubbles -->
    {"".join([
        f'<div class="bubble" style="width:{random.randint(30,80)}px; height:{random.randint(30,80)}px; background:{random.choice(colors)}; top:{random.randint(0,90)}%; left:{random.randint(0,90)}%;"></div>'
        for _ in range(10)
    ])}

    <div class="container">
        <div class="emoji">{emoji}</div>
        <h1>Python CGI Playground</h1>
        <p>{message}</p>
        <div class="time">Current server time: {current_time}</div>
        <div class="shape"></div>
        <div class="buttons">
            <a href="/" class="btn">Home</a>
            <a href="/python-cgi/test.py" class="btn">Refresh ✨</a>
        </div>
    </div>
</body>
</html>
""")