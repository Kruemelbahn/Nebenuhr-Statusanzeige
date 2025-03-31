# Nebenuhr-Statusanzeige

Nebenuhr-Statusanzeige is a Fastclock-slave, which is connected to the LocoNET on modelrailroad-layouts.<br>
This unit display the state of the connected-fastclock:
- blue = fastclock running in realtime-mode (clockrate 1:1)<br>
- green = fastclock running in fastclock-mode (= quicker than realtime)<br>
- red = fastclock stopped (not running)<br>
For more details please refer to the [manual](Documentation/Nebenuhr-Statusanzeige.pdf)<br>

### Basic Idea
The basic idea for developing Nebenuhr-Statusanzeige was taken from FREMO-magazine HP1 Modellbahn issue 1/2005 page 14 "Philipp Masmeier - Läuft sie oder läuft sie nicht?"

### Requested libraries
Nebenuhr-Statusanzeige requires my libraries listed below in addition to various Arduino standard libraries:<br> 
- [HeartBeat](https://www.github.com/Kruemelbahn/HeartBeat)
