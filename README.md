# Grive GUI

This is a GUI based on the [Grive2](https://github.com/vitalif/grive2), a Google Drive clients that syncs all the files in your Google Drive into the current directory. 

## Installation

### Install dependencies
You need to first install [Grive2](https://github.com/vitalif/grive2). For detailed instructions, see http://yourcmc.ru/wiki/Grive2#Installation

### Running GUI
```
git clone https://github.com/yanxi0830/grive-ui.git
cd grive-ui/
make
./grive-main-bin
```

## Usage
Choose the directory you want to sync with your Google Dirve and click Sync. Grive will start downloading files from your Google Drive to your directory. 

### Authentication
When the directory is used for the first time, you should click Authenticate to grant permission to Grive to access your Google Drive. A dialog should appear with a URL and prompt. You should go the the link and get an authentication code, and paste it into the prompt. 

If the authentication succeeded, Grive will create .grive and .grive_state files in your directory. You can then click Sync to download files from your Google Dirve to your local directory. 
