#!/usr/bin/env python3
import os
import sys
import base64
from urllib.parse import parse_qs

print("Content-Type: text/html")

# Check for logout request first
query_string = os.environ.get('QUERY_STRING', '')
if 'logout=1' in query_string:
    # Clear all session cookies
    cookie = os.environ.get("HTTP_COOKIE", "")
    if cookie:
        parts = [p.strip() for p in cookie.split(';')]
        for part in parts:
            if '=' in part:
                key, value = part.split('=', 1)
                if key.strip().startswith("session_"):
                    print(f"Set-Cookie: {key.strip()}=; Max-Age=0 ; path=/session")
    
    print()
    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Logged Out</title>
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }
        body {
            background-color: black;
            color: whitesmoke;
            font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
            text-align: center;
            padding: 2rem;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container {
            background-color: rgba(255, 255, 255, 0.05);
            padding: 50px;
            max-width: 500px;
            width: 100%;
            border-radius: 30px;
        }
        h1 {
            font-size: 50px;
            margin-bottom: 1.5rem;
        }
        p {
            font-size: 20px;
            margin-bottom: 2rem;
        }
        .login-link {
            display: inline-block;
            background-color: rgb(146, 3, 3);
            color: cornsilk;
            padding: 1rem 2rem;
            border-radius: 20px;
            text-decoration: none;
            font-size: 18px;
            font-weight: 600;
        }
        .login-link:hover {
            background-color: rgb(150, 40, 35);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Logged out</h1>
        <p>All sessions cleared.</p>
        <a href='/session/index.html' class="login-link">Login again</a>
    </div>
</body>
</html>""")
    exit()

cookie = os.environ.get("HTTP_COOKIE")
# Read form data from stdin
content_length = int(os.environ.get('CONTENT_LENGTH', 0))
form_data = sys.stdin.read(content_length)
# Parse the form data
parsed_data = parse_qs(form_data)
# Extract username and password
username = parsed_data.get('username', [''])[0]
password = parsed_data.get('password', [''])[0]

if cookie:
    user_cookie_name = f"session_{username}"
    found_session = ""
    stored_user = ""
    
    # Parse cookies to find session for this username
    parts = [p.strip() for p in cookie.split(';')]
    
    for part in parts:
        if '=' in part:
            key, value = part.split('=', 1)
            if key.strip() == user_cookie_name:
                try:
                    stored_user = base64.urlsafe_b64decode(value.strip().encode()).decode()
                    found_session = value.strip()
                    break
                except:
                    continue
    
    current_data = f"{username}:{password}"
    
    if stored_user == current_data:
        # Exact match - valid session
        print()  # end headers
        print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Welcome Back</title>
    <style>
        * {{
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }}
        body {{
            background-color: black;
            color: whitesmoke;
            font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
            text-align: center;
            padding: 2rem;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }}
        .container {{
            background-color: rgba(255, 255, 255, 0.05);
            padding: 50px;
            max-width: 700px;
            width: 100%;
            border-radius: 30px;
        }}
        h1 {{
            font-size: 50px;
            margin-bottom: 1rem;
        }}
        h3 {{
            font-size: 25px;
            margin: 2rem 0 1rem 0;
        }}
        .status {{
            background-color: rgb(3, 146, 3);
            color: white;
            padding: 0.5rem 1.5rem;
            border-radius: 20px;
            display: inline-block;
            margin-bottom: 2rem;
            font-weight: 600;
        }}
        .info-box {{
            background-color: rgba(255, 255, 255, 0.03);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            padding: 1.5rem;
            margin: 1rem 0;
            text-align: left;
        }}
        .info-item {{
            padding: 0.5rem 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }}
        .info-item:last-child {{
            border-bottom: none;
        }}
        .session-item {{
            background-color: rgba(255, 255, 255, 0.03);
            padding: 0.8rem;
            border-radius: 10px;
            margin: 0.5rem 0;
            font-family: 'Courier New', monospace;
            font-size: 14px;
        }}
        .logout-button {{
            background-color: rgb(146, 3, 3);
            color: cornsilk;
            padding: 1rem 2rem;
            border-radius: 20px;
            text-decoration: none;
            font-size: 18px;
            font-weight: 600;
            display: inline-block;
            margin-top: 2rem;
        }}
        .logout-button:hover {{
            background-color: rgb(150, 40, 35);
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Welcome back!</h1>
        <div class="status">AUTHENTICATED</div>
        
        <div class="info-box">
            <h3>Current Session</h3>
            <div class="info-item">User: {stored_user}</div>
            <div class="info-item">Cookie: {user_cookie_name}</div>
        </div>
        
        <div class="info-box">
            <h3>All Active Sessions</h3>""")
        
        # Show all existing sessions
        for part in parts:
            if '=' in part:
                key, value = part.split('=', 1)
                if key.strip().startswith("session_"):
                    try:
                        decoded = base64.urlsafe_b64decode(value.strip().encode()).decode()
                        print(f'            <div class="session-item">{key.strip()}: {decoded}</div>')
                    except:
                        pass

        print(f"""        </div>
        
        <a href='/session/log.py?logout=1' class="logout-button">Logout</a>
    </div>
</body>
</html>""")
        
    elif stored_user and stored_user != current_data:
        # Same username but different password - show error page
        print()  # end headers
        print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login Error</title>
    <style>
        * {{
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }}
        body {{
            background-color: black;
            color: whitesmoke;
            font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
            text-align: center;
            padding: 2rem;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }}
        .container {{
            background-color: rgba(255, 255, 255, 0.05);
            padding: 50px;
            max-width: 500px;
            width: 100%;
            border-radius: 30px;
        }}
        h1 {{
            font-size: 50px;
            margin-bottom: 1.5rem;
            color: rgb(200, 50, 50);
        }}
        .error-box {{
            background-color: rgba(146, 3, 3, 0.2);
            border: 2px solid rgb(146, 3, 3);
            padding: 1.5rem;
            border-radius: 15px;
            margin: 1.5rem 0;
            font-size: 18px;
        }}
        .user-info {{
            background-color: rgba(255, 255, 255, 0.03);
            padding: 1rem;
            border-radius: 10px;
            margin: 1rem 0;
            font-family: 'Courier New', monospace;
        }}
        .button {{
            display: inline-block;
            background-color: rgb(146, 3, 3);
            color: cornsilk;
            padding: 1rem 2rem;
            border-radius: 20px;
            text-decoration: none;
            font-size: 18px;
            font-weight: 600;
            margin: 0.5rem;
        }}
        .button:hover {{
            background-color: rgb(150, 40, 35);
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Login Failed</h1>
        <div class="error-box">
            Invalid credentials!<br>
            The username exists but the password is incorrect.
        </div>
        <div class="user-info">
            Username: {username}
        </div>
        <a href='/session/index.html' class="button">Try Again</a>
    </div>
</body>
</html>""")
        
    else:
        # No session found for this username - create new
        new_session_id = base64.urlsafe_b64encode(current_data.encode()).decode()
        print(f"Set-Cookie: {user_cookie_name}={new_session_id}")
        print()  # end headers
        print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>New Session</title>
    <style>
        * {{
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }}
        body {{
            background-color: black;
            color: whitesmoke;
            font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
            text-align: center;
            padding: 2rem;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }}
        .container {{
            background-color: rgba(255, 255, 255, 0.05);
            padding: 50px;
            max-width: 600px;
            width: 100%;
            border-radius: 30px;
        }}
        h1 {{
            font-size: 50px;
            margin-bottom: 1rem;
        }}
        .status {{
            background-color: rgb(3, 146, 3);
            color: white;
            padding: 0.5rem 1.5rem;
            border-radius: 20px;
            display: inline-block;
            margin-bottom: 2rem;
            font-weight: 600;
        }}
        .info-box {{
            background-color: rgba(255, 255, 255, 0.03);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            padding: 1.5rem;
            margin: 1.5rem 0;
            text-align: left;
        }}
        .info-item {{
            padding: 0.8rem 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }}
        .info-item:last-child {{
            border-bottom: none;
        }}
        .logout-button {{
            background-color: rgb(146, 3, 3);
            color: cornsilk;
            padding: 1rem 2rem;
            border-radius: 20px;
            text-decoration: none;
            font-size: 18px;
            font-weight: 600;
            display: inline-block;
            margin-top: 1rem;
        }}
        .logout-button:hover {{
            background-color: rgb(150, 40, 35);
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Session Created</h1>
        <div class="status">ACTIVE</div>
        
        <div class="info-box">
            <div class="info-item">User: {username}</div>
            <div class="info-item">Cookie: {user_cookie_name}</div>
            <div class="info-item">Session ID: {new_session_id[:20]}...</div>
        </div>
        
        <a href='/session/log.py?logout=1' class="logout-button">Logout</a>
    </div>
</body>
</html>""")

else:
    # No cookies at all
    data = f"{username}:{password}"
    session_id = base64.urlsafe_b64encode(data.encode()).decode()
    user_cookie_name = f"session_{username}"
    
    print(f"Set-Cookie: {user_cookie_name}={session_id}")
    print()  # end headers
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Welcome</title>
    <style>
        * {{
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }}
        body {{
            background-color: black;
            color: whitesmoke;
            font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
            text-align: center;
            padding: 2rem;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }}
        .container {{
            background-color: rgba(255, 255, 255, 0.05);
            padding: 50px;
            max-width: 600px;
            width: 100%;
            border-radius: 30px;
        }}
        h1 {{
            font-size: 50px;
            margin-bottom: 1rem;
        }}
        .status {{
            background-color: rgb(3, 146, 3);
            color: white;
            padding: 0.5rem 1.5rem;
            border-radius: 20px;
            display: inline-block;
            margin-bottom: 2rem;
            font-weight: 600;
        }}
        .info-box {{
            background-color: rgba(255, 255, 255, 0.03);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            padding: 1.5rem;
            margin: 2rem 0;
        }}
        h3 {{
            font-size: 25px;
            margin-bottom: 1rem;
        }}
        .cookie-display {{
            background-color: rgba(255, 255, 255, 0.03);
            padding: 1rem;
            border-radius: 10px;
            font-family: 'Courier New', monospace;
            font-size: 14px;
            word-break: break-all;
        }}
        .logout-button {{
            background-color: rgb(146, 3, 3);
            color: cornsilk;
            padding: 1rem 2rem;
            border-radius: 20px;
            text-decoration: none;
            font-size: 18px;
            font-weight: 600;
            display: inline-block;
            margin-top: 1rem;
        }}
        .logout-button:hover {{
            background-color: rgb(150, 40, 35);
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Welcome!</h1>
        <div class="status">FIRST SESSION</div>
        
        <div class="info-box">
            <h3>Session Details</h3>
            <div class="cookie-display">
                {user_cookie_name}={session_id}
            </div>
        </div>
        
        <a href='/session/log.py?logout=1' class="logout-button">Logout</a>
    </div>
</body>
</html>""")