#!/bin/bash

messages=(
    "Hello there, Webserv Explorer! 🚀"
    "Bash says hi! 🐚"
    "Webserv CGI test successful ✅"
    "Keep calm and code on 💻"
)

colors=("#007bff" "#28a745" "#ffc107" "#dc3545" "#17a2b8")

message=${messages[$RANDOM % ${#messages[@]}]}
btn_color=${colors[$RANDOM % ${#colors[@]}]}

cat <<EOF
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI Test - Bash Fun</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        body {
            background: linear-gradient(135deg, #f5f7fa, #c3cfe2);
            color: #333;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            text-align: center;
        }
        .container {
            background: rgba(255, 255, 255, 0.9);
            padding: 50px 80px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1);
            max-width: 600px;
        }
        h1 {
            font-size: 3rem;
            margin-bottom: 20px;
            color: #007bff;
        }
        p {
            font-size: 1.2rem;
            color: #555;
            margin-bottom: 30px;
        }
        .buttons {
            display: flex;
            justify-content: center;
            gap: 20px;
            margin-top: 20px;
        }
        .btn {
            padding: 15px 30px;
            font-size: 1.2rem;
            color: #fff;
            background-color: ${btn_color};
            border: none;
            border-radius: 5px;
            text-decoration: none;
            transition: background-color 0.3s ease;
        }
        .btn:hover {
            filter: brightness(0.85);
        }
        .emoji {
            font-size: 2rem;
            margin-bottom: 15px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="emoji">😎🎉🔥</div>
        <h1>Bash CGI Test</h1>
        <p>$message</p>
        <div class="buttons">
            <a href="/" class="btn">Home</a>
            <a href="/cgi-bin/test.sh" class="btn">Refresh ✨</a>
        </div>
    </div>
</body>
</html>
EOF
