#separator used by search.py, categories.py, ...
SEPARATOR = ';'

# Visit https://github.com/egirault/googleplay-api for instructions on how to configure this file
LANG            = 'en_US' # can be en_US, fr_FR, ...
ANDROID_ID      = None
GOOGLE_LOGIN    = None
GOOGLE_PASSWORD = None
AUTH_TOKEN      = None

# force the user to edit this file
if any([each == None for each in [ANDROID_ID, GOOGLE_LOGIN, GOOGLE_PASSWORD]]):
    raise Exception('config.py not updated')

