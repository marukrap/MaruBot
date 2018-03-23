# MaruBot

A Twitch chatbot written in C++/SFML with IRC interface.

## How to use

### 1. Create OAuth Password

[http://twitchapps.com/tmi/](http://twitchapps.com/tmi/)

### 2. Create Config.txt file

```
oauth:oauth_password
twitch_username
#twitch_channel
```

### 3. Application

Press **Esc key** to close the application when the window is active. (Click the taskbar icon to activate the window before press the key. if the key doesn't work, deactivate and reactivate the window again)

* **!quit** - Close the application gracefully
* **!addbot [bot name]** - Add a bot
* **!removebot [bot name]** - Remove a bot

### 4. List of bots

#### Jukebox

Put [youtube-dl](https://rg3.github.io/youtube-dl/) and [mpv](https://mpv.io/) in **/Jukebox** folder and add your music files to **/Jukebox/Music**. You must have youtube-dl.exe and mpv.com in **/Jukebox** folder. (Available formats: mp3, mp4, ogg, flac, webm)

* **!currentsong/!cs** - Display the title of the current song
* **!prevsong/!ps** - Display the title of the previous song
* **!songrequest/!sr [a link or song title]** - Request a song
* **!skip** - Skip the current song (default song only)

#### AnswerBot

You can create **Answers.txt** or use commands below.

```
!question1 = answer1
!question2 = answer2
!question3 = answer3
```

* **!answer [question] [answer]** - Add a new command
* **!answer [question]** - Remove a command

#### LixBot

* **!join** - Join the game
* **!part** - Depart from the game
* **!walk** - Walk back or cancel blockers
* **!jump** - Jump once. Runners jump further
* **!run** - Run faster than walkers
* **!climb** - Climb all vertical walls
* **!float** - Float to survive all falls
* **!bat** - Bat other lix, flinging them
* **!implode** - Implode to destroy earth/cancel blockers
* **!explode** - Explode to fling other lix
* **!block** - Block other lix, turning them
* **!cube** - Cube to generate earth quickly
* **!build** - Build a diagonal staircase
* **!platform** - Platform a horizontal bridige
* **!bash** - Bash a horizontal tunnel
* **!mine** - Mine a diagonal tunnel
* **!dig** - Dig a vertical hole

#### BattlepusBot

You can drag and drop a character with **Ctrl key** while the window is active. you can also change a character at the mouse position by pressing **Alt key**.

* **!join** - Join the game
* **!join #** - Join # characters ex) !join 10 *(admin only)*
* **!join all** - Join all available characters with their own name *(admin only)*
* **!part** - Depart from the game
* **!part all** - Remove all of your characters *(admin only)*
* **!change** - Change your character
* **!change [name id]** - Replace your character with a character of that name id (a filename without extension)
* **!s/!sw/!w/!nw/!n/!ne/!e/!se** - Change the direction of your character
* **!alpha #** - Change the transparency of characters [0-255]
* **!scale #** - Change the size of characters [1-4]

## Credit

* [Lix](http://www.lixgame.com/)
