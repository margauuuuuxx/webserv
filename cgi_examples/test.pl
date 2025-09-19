#!/usr/bin/perl
use strict;
use warnings;
use CGI;

# Create a new CGI object
my $cgi = CGI->new;

# Set the Content-Type header
print $cgi->header(-type => 'text/html', -charset => 'UTF-8');

# Get parameters
my $name = $cgi->param('name') || 'Anonymous Adventurer';
my $color = $cgi->param('color') || 'blue';

# Fun messages
my @messages = (
    "Welcome to the magical Perl CGI kingdom!",
    "Brace yourself, the dragons are coming 🐉",
    "Have a cookie 🍪 and enjoy your stay!",
    "Rainbows are free today 🌈",
    "You have chosen wisely, brave traveler ⚔️",
);

# Pick a random message
my $message = $messages[int(rand(@messages))];

# HTML Output
print <<"HTML";
<!DOCTYPE html>
<html>
<head>
    <title>Perl CGI Playground</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            text-align: center;
            padding: 30px;
        }
        h1 {
            color: $color;
        }
        .box {
            background: white;
            border-radius: 10px;
            padding: 20px;
            display: inline-block;
            box-shadow: 0 0 10px rgba(0,0,0,0.2);
        }
        .env {
            font-size: 14px;
            text-align: left;
            margin-top: 20px;
            max-width: 500px;
            overflow-x: auto;
        }
    </style>
</head>
<body>
    <div class="box">
        <h1>Hello, $name!</h1>
        <p>$message</p>
        <form method="GET">
            <label>Your Name: <input type="text" name="name"></label><br><br>
            <label>Favorite Color: <input type="text" name="color" placeholder="blue"></label><br><br>
            <button type="submit">Refresh</button>
        </form>

        <h2>Environment Variables</h2>
        <div class="env">
HTML

# Print environment variables
foreach my $key (sort keys %ENV) {
    print "<b>$key:</b> $ENV{$key}<br>\n";
}

print <<"HTML";
        </div>
    </div>
</body>
</html>
HTML
