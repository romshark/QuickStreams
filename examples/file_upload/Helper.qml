import QtQuick 2.0

Item {
	id: helper

	// Helper function for random number generation
	property var randomBetween: function(min, max) {
		return Math.floor(Math.random() * (max - min + 1)) + min
	}

	// Helper function for delayed, queued code execution
	property var setTimeout: function(delayMillisec, callbackFunction) {
		var timer = Qt.createQmlObject(
			'import QtQuick 2.7; Timer {}',
			helper
		)
		timer.interval = delayMillisec
		timer.repeat = false
		var callback = function() {
			callbackFunction()
			timer.triggered.disconnect(callback)
		}
		timer.triggered.connect(callback)
		timer.start()
	}

	// Helper function for random identifier string generation
	property var randomId: function(length) {
		var text = '';
		var possible = 'ABCDEF0123456789';
		for(var i = 0; i < length; i++) {
			text += possible.charAt(
				Math.floor(Math.random() * possible.length)
			)
		}
		return text
	}

	// Helper function for random network latency duration generation
	property var randomNetLatency: function() {
		return helper.randomBetween(minNetworkLatency, maxNetworkLatency)
	}
}
