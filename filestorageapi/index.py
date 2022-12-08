import jwt

from flask import Flask, request, make_response, jsonify, Response

from storage import upload_to_minio, get_file
from variables import SECRET

app = Flask(__name__)
PORT = 3200
HOST = "0.0.0.0"


@app.route("/upload", methods=["POST"])
def upload():
    if "file" not in request.files or "email" not in request.form:
        return make_response(jsonify({"error": "Missing arguments: file or email"}), 400)

    res = []
    for file in request.files.getlist("file"):
        url = upload_to_minio(file, request.form["email"])
        res.append(url)
    return make_response(res)


@app.route("/file/<file_name>", methods=["GET"])
def get(file_name):
    if "sender" not in request.args or "token" not in request.args:
        return make_response(jsonify({"error": "Missing query arguments: sender or token"}), 400)

    filename = file_name
    email = request.args["sender"]
    token = request.args["token"]
    try:
        infos = jwt.decode(token, SECRET, algorithms=["HS256"])
    except Exception as err:
        print(err)
        return make_response(jsonify({"error": "Wrong token"}), 400)
    file = get_file(email, filename, infos)
    if file.status_code == 200:
        return Response(file.content, mimetype=file.headers.get("Content-Type"), status=200)
    else:
        return make_response(jsonify({"error": "Unable to retrieve the file"}), 404)


if __name__ == "__main__":
    app.run(host=HOST, port=PORT)
