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
        "name" : "closeSwarmButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : .2, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "Close swarm",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"B\");",
			"isLocal": true,
    },
    {
        "name" : "farSwarmButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : .3, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "Far swarm",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"N\");",
			"isLocal": true,
    },
    {
        "name" : "fieldButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : .4, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "Toggle field",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"V\");",
			"isLocal": true,
    },

    {
        "name" : "panNormalButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Pan Normal",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"1\");",
			"isLocal": true,
    },
    {
        "name" : "panFast1Button",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.1, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Pan Fast 1",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"2\");",
			"isLocal": true,
    },
    {
        "name" : "panFast2Button",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.2, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Pan Fast 2",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"3\");",
			"isLocal": true,
    },
    {
        "name" : "panUpButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.3, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Pan Up",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"5\");",
			"isLocal": true,
    },
    {
        "name" : "fadeButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.4, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "PanUp Fade",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"4\");",
			"isLocal": true,
    },
    {
        "name" : "zoomButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.5, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "FastZ",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"6\");",
			"isLocal": true,
    },
    {
        "name" : "resetButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.6, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Reset",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"0\");",
			"isLocal": true,
    },
    {
        "name" : "stopButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.7, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Stop",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"9\");",
			"isLocal": true,
    },
    {
        "name" : "blurButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.8, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Blur",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"7\");",
			"isLocal": true,
    },
    {
        "name" : "sharpButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.9, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Sharpen",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"8\");",
			"isLocal": true,
    },

    {
        "name" : "dropButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.0, "y" : 0.4,
        "width" : .1, "height" : .1,
        "label" : "Drop",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"M\");",
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
	     "y": .90,
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

