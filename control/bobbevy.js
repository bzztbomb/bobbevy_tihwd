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
        "mode": "toggle",
        "x" : .1, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "Toggle trees",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"T\");",
			"isLocal": true,
    },
    {
        "name" : "treeBlurButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : .1, "y" : .1,
        "width" : .1, "height" : .1,
        "label" : "TreesLeaves",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"fE\");",
			"isLocal": true,
    },
    {
        "name" : "closeSwarmButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : .2, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "Close swarm",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"B\");",
			"isLocal": true,
    },
    {
        "name" : "farSwarmButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : .3, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "Far swarm",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"N\");",
			"isLocal": true,
    },
    {
        "name" : "fieldButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : .4, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "Toggle field",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"V\");",
			"isLocal": true,
    },
    {
        "name" : "blackButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : .5, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "BlackFade",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"D\");",
			"isLocal": true,
    },
    {
        "name" : "whiteButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : .5, "y" : 0.1,
        "width" : .1, "height" : .1,
        "label" : "WhiteFade",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"C\");",
			"isLocal": true,
    },
    {
        "name" : "inButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : .6, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "FadeIn",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"I\");",
			"isLocal": true,
    },
    {
        "name" : "outButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : .7, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "FadeOut",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"O\");",
			"isLocal": true,
    },
    {
        "name" : "inSlowButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : .8, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "FadeInSlow",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"G\");",
			"isLocal": true,
    },
    {
        "name" : "inSlowButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : .9, "y" : 0,
        "width" : .1, "height" : .1,
        "label" : "Reset timeline",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"H\");",
			"isLocal": true,
    },

    {
        "name" : "panNormalButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : 0, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Pan Normal",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"1\");",
			"isLocal": true,
    },
    {
        "name" : "panFast1Button",
        "type" : "Button",
        "mode": "toggle",
        "x" : 0.1, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Pan Fast 1",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"2\");",
			"isLocal": true,
    },
    {
        "name" : "panFast2Button",
        "type" : "Button",
        "mode": "toggle",
        "x" : 0.2, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Pan Fast 2",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"3\");",
			"isLocal": true,
    },
    {
        "name" : "panUpButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : 0.3, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Pan Up",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"5\");",
			"isLocal": true,
    },
    {
        "name" : "fadeButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : 0.4, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "PanUp Fade",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"4\");",
			"isLocal": true,
    },
    {
        "name" : "zoomButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : 0.5, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "FastZ",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"6\");",
			"isLocal": true,
    },
    {
        "name" : "resetButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : 0.6, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Reset",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"0\");",
			"isLocal": true,
    },
    {
        "name" : "stopButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : 0.7, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Stop",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"9\");",
			"isLocal": true,
    },
    {
        "name" : "blurButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : 0.8, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Blur",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"7\");",
			"isLocal": true,
    },
    {
        "name" : "sharpButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : 0.9, "y" : 0.2,
        "width" : .1, "height" : .1,
        "label" : "Sharpen",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"8\");",
			"isLocal": true,
    },

    {
        "name" : "dropButton",
        "type" : "Button",
        "mode": "toggle",
        "x" : 0.0, "y" : 0.4,
        "width" : .1, "height" : .1,
        "label" : "Drop",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"M\");",
			"isLocal": true,
    },

    {
        "name" : "lightToggleButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.0, "y" : 0.6,
        "width" : .2, "height" : .1,
        "label" : "Disable Kinect",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"7\");",
			"isLocal": true,
    },
    {
        "name" : "revealButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.2, "y" : 0.6,
        "width" : .2, "height" : .1,
        "label" : "Enable Kinect",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"L\");",
			"isLocal": true,
    },
    {
        "name" : "revealButton",
        "type" : "Button",
        "mode": "momentary",
        "x" : 0.4, "y" : 0.6,
        "width" : .2, "height" : .1,
        "label" : "Reset depth",
        "ontouchstart" : "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"A\");",
			"isLocal": true,
    },

    {
        "name": "refreshButton",
        "type": "Button",
        "x": 0,
        "y": .90,
        "width": .15,
        "height": .10,
        "mode": "toggle",
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
	 {
	     "name": "CueButton",
	     "type": "Button",
	     "x": .30,
	     "y": .90,
	     "width": .145,
	     "height": .10,
	     "mode":"toggle",
	     "colors": ["#000", "#444", "#aaa"],
	     "ontouchstart": "control.changePage(1);",
	     "label": "cues",
			"isLocal": true,
	 },
	 {
	     "name": "MultiButton",
	     "type": "Button",
	     "x": .40,
	     "y": .90,
	     "width": .145,
	     "height": .10,
	     "mode":"toggle",
	     "colors": ["#000", "#444", "#aaa"],
	     "ontouchstart": "control.changePage(2);",
	     "label": "multi",
			"isLocal": true,
	 },
],
[
    {
        "name": "infoText",
        "type": "Label",
        "x": 0, "y": 0,
        "width": 0.4, "height": .1,
        "value": "Jessica solo, start with black and trees disabled.",
    },
    {
        "name": "fadeOut",
        "type": "Button",
        "x": 0.4,
        "y": .0,
        "width": .10,
        "height": .10,
        "mode": "toggle",
	     "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"O\");",
        "label":"Start",
			"isLocal": true,
    },
    {
        "name": "fadeInSlowNoMove",
        "type": "Button",
        "x": 0.50,
        "y": .0,
        "width": .15,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"E\"); oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"9\");oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"W\");oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"G\");",
        "label":"FadeInSlowNoMove",
			"isLocal": true,
    },
    {
        "name": "lessFog",
        "type": "Button",
        "x": 0.65,
        "y": .0,
        "width": .15,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"R\");",
        "label":"LessFog",
			"isLocal": true,
    },
    {
        "name": "startMove",
        "type": "Button",
        "x": 0.8,
        "y": .0,
        "width": .15,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"1\");oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"U\");",
        "label":"Move",
			"isLocal": true,
    },

    {
        "name": "infoText",
        "type": "Label",
        "x": 0, "y": 0.1,
        "width": 1, "height": .1,
        "value": "Caldera, starts with chest tap, trees are hopefully faded in by now",
    },
    {
        "name": "fastZ",
        "type": "Button",
        "x": 0,
        "y": .2,
        "width": .30,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"6\");",
        "label":"FastZ when dancers face screen",
			"isLocal": true,
    },
    {
        "name": "stop",
        "type": "Button",
        "x": .30,
        "y": .2,
        "width": .30,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"S\");",
        "label":"StopOnMove",
			"isLocal": true,
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .60,
        "y": .2,
        "width": .30,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"1\");",
        "label":"MoveAgain",
			"isLocal": true,
    },

    {
        "name": "infoText",
        "type": "Label",
        "x": 0, "y": 0.27,
        "width": 1, "height": .1,
        "value": "Conversation.",
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .5,
        "y": .27,
        "width": .30,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"8\");",
        "label":"Walk on diag, go into leaves",
			"isLocal": true,
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .0,
        "y": .35,
        "width": .30,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"J\");",
        "label":"Activate swarms (during egyptians)",
			"isLocal": true,
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .45,
        "y": .35,
        "width": .20,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"D\");oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"O\");",
        "label":"Fade out trees (on shake)",
			"isLocal": true,
    },

    {
        "name": "infoText",
        "type": "Label",
        "x": 0, "y": 0.43,
        "width": 1, "height": .1,
        "value": "March (music shift, spin walk)",
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .0,
        "y": .5,
        "width": .30,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"M\");oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"V\");oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"T\");",
        "label":"Step on diag, field",
			"isLocal": true,
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .3,
        "y": .5,
        "width": .30,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"T\");oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"I\");",
        "label":"FadeTreesOnJazzHandsHandClasp",
			"isLocal": true,
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .6,
        "y": .5,
        "width": .30,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"M\");",
        "label":"DropFieldOnChoke",
			"isLocal": true,
    },
    {
        "name": "infoText",
        "type": "Label",
        "x": 0, "y": 0.57,
        "width": 1, "height": .1,
        "value": "Pitchfork (you know this bitz!)",
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .0,
        "y": .65,
        "width": .15,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"2\");",
        "label":"PanFast1",
			"isLocal": true,
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .15,
        "y": .65,
        "width": .15,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"1\");",
        "label":"PanNormal",
			"isLocal": true,
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .30,
        "y": .65,
        "width": .15,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"3\");",
        "label":"PanFast2",
			"isLocal": true,
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .45,
        "y": .65,
        "width": .15,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"5\");",
        "label":"PanUp (chokespin)",
			"isLocal": true,
    },
    {
        "name": "moveAgain",
        "type": "Button",
        "x": .6,
        "y": .65,
        "width": .15,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
        "stroke": "#aaaaaa",
        "ontouchstart": "oscManager.sendOSC(\"/bobbevy/key\", \"s\", \"4\");",
        "label":"FadeOut",
			"isLocal": true,
    },

    {
        "name": "refreshButton",
        "type": "Button",
        "x": 0,
        "y": .90,
        "width": .15,
        "height": .10,
        "mode": "toggle",
        "colors": ["#000", "#444", "#aaa"],
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
	 {
	     "name": "rawButton",
	     "type": "Button",
	     "x": .30,
	     "y": .90,
	     "width": .145,
	     "height": .10,
	     "mode":"toggle",
	     "colors": ["#000", "#444", "#aaa"],
	     "ontouchstart": "control.changePage(0);",
	     "label": "raw",
			"isLocal": true,
	 },
],
[
	 {
	     "name": "multi",
	     "type": "MultiTouchXY",
	     "x": .0,
	     "y": .0,
	     "width": .8,
	     "height": .8,
        "maxTouches": 4,
        "numberOfTouches": 4,
        "isMomentary": false,
	     "colors": ["#000", "#444", "#aaa"],
	 },

	 {
	     "name": "MultiButton",
	     "type": "Button",
	     "x": .40,
	     "y": .90,
	     "width": .145,
	     "height": .10,
	     "mode":"toggle",
	     "colors": ["#000", "#444", "#aaa"],
	     "ontouchstart": "control.changePage(0);",
	     "label": "raw",
			"isLocal": true,
	 },
	 {
	     "name": "MultiButton",
	     "type": "Button",
	     "x": .50,
	     "y": .90,
	     "width": .145,
	     "height": .10,
	     "mode":"toggle",
	     "colors": ["#000", "#444", "#aaa"],
	     "ontouchstart": "window.multi.addTouch(0,0,window.multi.children.length);",
	     "label": "add",
			"isLocal": true,
	 },
],
];

