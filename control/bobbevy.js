loadedInterfaceName = "bobbevy";
interfaceOrientation = "portrait";

pages = [ [
    {
        "name" : "lightButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "Toggle light",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"L\");",
			"isLocal": true,
    },
    {
        "name" : "treeButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : .1, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "Toggle trees",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"T\");",
			"isLocal": true,
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
        "label":"Refresh",
			"isLocal": true,
    },
	 {
	     "name": "tabButton",
	     "type": "Button",
	     "x": .15,
	     "y": .80,
	     "width": .145,
	     "height": .10,
	     "mode":"toggle",
	     "colors": ["#000", "#444", "#aaa"],
	     "ontouchstart": "control.showToolbar();",
	     "label": "menu",
			"isLocal": true,
	 },

],

];

