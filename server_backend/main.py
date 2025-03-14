import sqlite3
from flask import Flask, request, jsonify
from flask_jwt_extended import (
    JWTManager, create_access_token, 
    jwt_required, get_jwt_identity
)
from werkzeug.security import generate_password_hash, check_password_hash
from datetime import timedelta, datetime

app = Flask(__name__)

# Secret key for signing JWTs (rotate this regularly for added security)
app.config["JWT_SECRET_KEY"] = "super_secret_key"  # Change this in production!
app.config["JWT_ACCESS_TOKEN_EXPIRES"] = timedelta(minutes=5)  # 5-min session

jwt = JWTManager(app)

# SQLite Database setup
DATABASE = 'demonized.db'
VERSION = "d1.0.2"

def check_useragent(ua):
    if (ua == "dmnzd_frontend/1.0"):
        return True

    return False

def init_db():
    with sqlite3.connect(DATABASE) as conn:
        cursor = conn.cursor()
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS logs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                uid INTEGER NOT NULL,
                event_type TEXT NOT NULL,
                event_details TEXT,
                ip_address TEXT,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (uid) REFERENCES users(uid) ON DELETE CASCADE
            );
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS users (
                uid INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL UNIQUE,
                password TEXT NOT NULL,
                email TEXT UNIQUE,
                banned BOOLEAN DEFAULT FALSE,
                ban_reason TEXT
            );
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS products (
                pid INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                status TEXT NOT NULL
            );
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS subscriptions (
                uid INTEGER,
                pid INTEGER,
                expiration_date TEXT,
                FOREIGN KEY (uid) REFERENCES users (uid),
                FOREIGN KEY (pid) REFERENCES products (pid)
            );
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS keys (
                kid INTEGER PRIMARY KEY AUTOINCREMENT,
                key TEXT NOT NULL UNIQUE,
                pid INTEGER NOT NULL,  -- Product ID associated with the key
                time INTEGER NOT NULL,  -- Time in days the user gets access to the product
                claimed INTEGER DEFAULT 0,  -- 0 means not claimed, uid means claimed by the user
                FOREIGN KEY (pid) REFERENCES products (pid)  -- Link to the products table
            );
        """)
        conn.commit()

init_db()

print(generate_password_hash("1234"))
#@app.route("/register", methods=["POST"])
#def register():
#    data = request.json
#    username = data.get("username")
#    password = data.get("password")
#
#    if not username or not password:
#        return jsonify({"msg": "Username and password are required"}), 400
#
#    # Hash the password before storing it
#    hashed_password = generate_password_hash(password)
#
#    # Insert new user into the users table
#    with sqlite3.connect(DATABASE) as conn:
#        cursor = conn.cursor()
#        try:
#            cursor.execute("INSERT INTO users (username, password) VALUES (?, ?)", (username, hashed_password))
#            conn.commit()
#            return jsonify({"msg": "User registered successfully!"}), 201
#        except sqlite3.IntegrityError:
#            return jsonify({"msg": "Username already exists"}), 400

@app.route("/login", methods=["POST"])
def login():
    if not (check_useragent(request.headers.get('User-Agent'))):
        return jsonify({"msg": "nigga what"}), 404

    data = request.json
    username = data.get("username").lower()
    password = data.get("password")

    # Verify user credentials from the database
    with sqlite3.connect(DATABASE) as conn:
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM users WHERE username = ?", (username,))
        user = cursor.fetchone()
        if not user:
            return jsonify({"msg": "Invalid credentials"}), 401

        if user[4] == True: # Check if user is banned
            return jsonify({"msg": f"Sadly you can not use the software due to an active ban.\nBan reason - {user[5]}"}), 201
            
        if check_password_hash(user[2], password):  # user[2] is the hashed password
            access_token = create_access_token(identity=user[0])  # user[0] is uid
            return jsonify(access_token=access_token)
    
    return jsonify({"msg": "Invalid credentials"}), 401

@app.route("/get_version", methods=["GET"])
def get_version():
    if not (check_useragent(request.headers.get('User-Agent'))):
        return jsonify({"msg": "nigga what"}), 404

    return jsonify({"version": VERSION}), 200

#@app.route("/refresh", methods=["POST"])
#@jwt_required(refresh=True)
#def refresh():
#    identity = get_jwt_identity()
#    new_access_token = create_access_token(identity=identity)
#    return jsonify(access_token=new_access_token)

@app.route("/products", methods=["GET"])
@jwt_required()
def products():
    if not (check_useragent(request.headers.get('User-Agent'))):
        return jsonify({"msg": "nigga what"}), 404
        
    user = get_jwt_identity()

    with sqlite3.connect(DATABASE) as conn:
        cursor = conn.cursor()
        cursor.execute("""
            SELECT p.pid, p.name, p.status, s.expiration_date 
            FROM products p
            LEFT JOIN subscriptions s ON p.pid = s.pid AND s.uid = ?
            """, (user,))
        
        products = []
        for row in cursor.fetchall():
            pid, name, status, expiration_date = row
            expiration_date = datetime.strptime(expiration_date, "%Y-%m-%d %H:%M:%S") if expiration_date else None
            if expiration_date is None:
                continue
                
            status = status if status else "Unknown"
            is_expired = expiration_date and expiration_date < datetime.now()

            if not is_expired:
                products.append({
                    "pid": pid,
                    "name": name,
                    "status": status,
                    "expiration_date": expiration_date.strftime("%Y-%m-%d %H:%M:%S") if expiration_date else "N/A"
                })

    return jsonify(products), 200

@app.route("/claim_key", methods=["POST"])
@jwt_required()
def claim_key():
    if not (check_useragent(request.headers.get('User-Agent'))):
        return jsonify({"msg": "nigga what"}), 404
        
    user = get_jwt_identity()

    data = request.json
    key = data.get("key")

    if not key:
        return jsonify({"msg": "Key is required"}), 400

    with sqlite3.connect(DATABASE) as conn:
        cursor = conn.cursor()

        # Check if the key is valid and unclaimed
        cursor.execute("SELECT * FROM keys WHERE key = ? AND claimed = 0", (key,))
        result = cursor.fetchone()

        if not result:
            return jsonify({"msg": "Invalid key or already claimed"}), 400

        kid, key_value, pid, time_days, claimed = result

        # Calculate expiration date (current date + time_days)
        expiration_date = datetime.now() + timedelta(days=time_days)
        expiration_date_str = expiration_date.strftime("%Y-%m-%d %H:%M:%S")

        # Update the keys table to mark the key as claimed by the user
        cursor.execute("UPDATE keys SET claimed = ? WHERE key = ?", (user, key))
        
        # Check if the user already has a subscription for the product
        cursor.execute("SELECT * FROM subscriptions WHERE uid = ? AND pid = ?", (user, pid))
        subscription = cursor.fetchone()

        if subscription:
            # If the user already has a subscription, just update the expiration date
            cursor.execute("UPDATE subscriptions SET expiration_date = ? WHERE uid = ? AND pid = ?",
                           (expiration_date_str, user, pid))
        else:
            # If no subscription exists, create a new one
            cursor.execute("INSERT INTO subscriptions (uid, pid, expiration_date) VALUES (?, ?, ?)",
                           (user, pid, expiration_date_str))

        conn.commit()

    return jsonify({"msg": "Key claimed successfully!"}), 200

@app.route("/user_data", methods=["GET"])
@jwt_required()
def user_data():
    if not (check_useragent(request.headers.get('User-Agent'))):
        return jsonify({"msg": "nigga what"}), 404
        
    user = get_jwt_identity()

    # Fetch user data
    with sqlite3.connect(DATABASE) as conn:
        cursor = conn.cursor()
        cursor.execute("SELECT username FROM users WHERE uid = ?", (user,))
        user_info = cursor.fetchone()

        if user_info:
            return jsonify({"user": user_info[0]})
    
    return jsonify({"msg": "User not found"}), 404

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000, debug=True)