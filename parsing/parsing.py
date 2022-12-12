import email
import sys
import base64
import requests

URL_TO_FILE_SERVER = "http://filestorageapi:3200"


def send_file_server(file_info, sender):
    data = base64.b64decode(file_info["content"])
    files = {"file": (file_info["filename"], data, file_info["type"])}
    try:
        link = requests.post(URL_TO_FILE_SERVER + "/upload", data={"email": sender.strip()}, files=files)
    except requests.exceptions.ConnectionError as e:
        print("Can't connect to " + URL_TO_FILE_SERVER)
        print(e)
        return
    if link.status_code == 200:
        file_info["type"] = "application/txt"
        file_info["filename"] = file_info["filename"].split(".")[0] + "_link.txt"
        file_info["content"] = str(
            base64.b64encode((URL_TO_FILE_SERVER + link.json()[0]).encode('utf-8')).decode('utf-8')) + "\n"
    else:
        return


def parse_mime_files(mime_message):
    mime = get_mime_from_string(mime_message)
    if not is_mime_message(mime): return mime.as_string()

    sender = mime.get('From', "failed@mail.com")
    parts = mime.get_payload()
    for part in parts:
        if None == part.get('Content-disposition') or "attachment" not in part.get('Content-disposition'):
            continue

        file_info = {"filename": part.get_filename(), "content": part.get_payload(), "type": part.get_content_type()}

        send_file_server(file_info, sender)
        part.replace_header('Content-disposition', "attachment; filename=\"" + file_info["filename"] + '"')
        part.replace_header('Content-type', "text/plain; charset=UTF-8; name=\"" + file_info["filename"] + '"')
        part.set_payload(file_info["content"])
    return mime.as_string()


def is_mime_message(mime_message):
    return mime_message.is_multipart()


def get_mime_from_string(message):
    return email.message_from_string(message)


def get_boundary_without_header(message):
    return message.split("\n")[1][2:]


# Voir exemple postfix_message_raw (exemple de ce qui est reçu par le filtre)
def format_body_without_header(message, sender):
    boundary = get_boundary_without_header(message)
    new_message = "Content-Type: multipart/mixed; boundary=" + boundary + "\n"
    new_message += "From: " + sender + "\n" + message
    return new_message


# enleve les headers ajoutés pour le parsing du message
def deformat_headers(message):
    return "\n".join(message.split("\n")[2:])


# Tester le parsing avec un fichier
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Please provide the path to the file to test in parameters")
        exit(0)
    with open(sys.argv[1], 'r') as file:
        msg = file.read()
        file.close()
    msg = format_body_without_header(msg, "test2@mail.fr")
