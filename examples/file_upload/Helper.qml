import QtQuick 2.0

Item {
	id: helper

	// Helper function for random number generation
	property var randomBetween: function(min, max) {
		return Math.floor(Math.random() * (max - min + 1)) + min
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
