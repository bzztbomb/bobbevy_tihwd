loadedInterfaceName = "simple";
interfaceOrientation = "portrait";

pages = [ [
{
    "name" : "lightButton",
    "type" : "Button",
    "mode": "momentary",
    "x" : 0, "y" : 0,
    "width" : .1, "height" : .1,
    "label" : "Disable light",
    "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"L\");"
},
{
"name": "refreshButton",
"type": "Button",
"x": 0,
"y": .90,
"width": .15,
"height": .10,
"mode": "momentary",
"color": "#000000",
"stroke": "#aaaaaa",
"ontouchstart": "interfaceManager.refreshInterface()",
    "label":"Refresh"
}
],

];

