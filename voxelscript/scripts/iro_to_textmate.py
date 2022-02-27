from selenium import webdriver
import sys
import time

# Get stdin
iro_data = ''.join(sys.stdin.readlines())

# Create the selenium driver
options = webdriver.firefox.options.Options()
options.headless = True
driver = webdriver.Firefox(options=options)

# Navigate to iro
driver.get(sys.argv[1])
time.sleep(2)

# Click the I Agree button
driver.execute_script("document.getElementsByClassName('rionx_button')[0].click();")
time.sleep(0.25)

# Set the content of the iro
driver.execute_script("""
function set_content(id, text) {
	var aceEditorElement = document.getElementsByClassName('ace_editor')[id].id;
	var editorContainer = document.getElementById(aceEditorElement);
	var editor = ace.edit(editorContainer);
	editor.session.setValue(text);
}
set_content(0, arguments[0])
""", iro_data)
time.sleep(0.25)

# Click the play button
driver.execute_script("document.getElementsByClassName('fa-play')[0].click()")
time.sleep(1)

# Get the content from textmate
result = driver.execute_script("""
function get_content(id) {
	var aceEditorElement = document.getElementsByClassName('ace_editor')[id].id;
	var editorContainer = document.getElementById(aceEditorElement);
	var editor = ace.edit(editorContainer);
	return editor.session.getValue();
}
return get_content(2);
""")

print(result)

driver.close()
