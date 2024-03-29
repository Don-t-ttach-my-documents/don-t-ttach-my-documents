from parsing import parse_mime_files, is_mime_message, format_body_without_header, deformat_headers, \
    get_mime_from_string
from flask import Flask, request, make_response

app = Flask(__name__)
PORT = 3201
HOST = "0.0.0.0"


@app.route("/upload", methods=["POST"])
def upload():
    message = request.get_data().decode("utf-8")
    message_mime = get_mime_from_string(message)
    if not is_mime_message(message_mime):
        # Voir exemple postfix_message_raw.txt
        if "This is a multi-part message in MIME format." in message:
            message = parse_mime_files(format_body_without_header(message, request.headers.get("Sender")))
            res = deformat_headers(message)
        else:
            # Si le message n'est pas au format MIME, renvoyer le contenu
            res = message
    else:
        res = parse_mime_files(message)
    return make_response(res, 200)


if __name__ == "__main__":
    app.run(host=HOST, port=PORT)
