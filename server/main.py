from flask import Flask, request, render_template
from flask_json import FlaskJSON, as_json, json_response, JsonError
from flask_cors import CORS

displayed_text = "HELLO"

app = Flask(__name__)
CORS(app)
json = FlaskJSON(app)

@app.route("/")
def root():
    return render_template('index.html', displayed_text=displayed_text)


@app.route("/get_text")
def get_text():
    global displayed_text
    print(displayed_text)
    return json_response(text=displayed_text)

@app.route("/set_text", methods=['POST'])
def set_text():
    global displayed_text
    print("/set_text")
    if request.method == "POST":
        data = request.get_json(force=True)
        try:
            print("Data:", data)
            displayed_text = data['text']
            print("Value:", displayed_text)
        except (KeyError, TypeError, ValueError):
            raise JsonError(description="Invalid value.")
    return json_response(200)


def main():
    print("Starting server")

main()
