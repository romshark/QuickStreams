import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.1

// Upload list item delegate
Rectangle {
	id: root
	RowLayout {
		Layout.fillWidth: true
		anchors.fill: parent
		anchors.leftMargin: 16
		anchors.rightMargin: 16
		spacing: 8
		Row {
			spacing: 8
			Layout.minimumWidth: 64
			Layout.alignment: Qt.AlignLeft
			Text {
				text: identifier.length > 1 ? identifier : '- - - - - - - -'
				width: 64
				horizontalAlignment: Text.AlignHCenter
			}
		}
		RowLayout {
			spacing: 8
			Text {
				visible: step != 'uploading'
				anchors.verticalCenter: parent.verticalCenter
				Layout.alignment: Qt.AlignLeft
				Layout.fillWidth: true
				text: switch(step) {
					case 'aborted': return 'Upload was aborted'
					case 'finished': return 'Successfuly Uploaded'
					case 'failed': return 'Upload Failed: ' + failureReason
					case 'preparing': return 'Preparing...'
					case 'aborting': return 'Aborting...'
					default: return ''
				}
			}
			ProgressBar {
				Layout.fillWidth: true
				Layout.alignment: Qt.AlignLeft
				visible: step == 'uploading'
				to: size
				value: progress
			}
			Text {
				Layout.alignment: Qt.AlignRight
				visible: step == 'uploading'
				text: progress + ' / ' + size
			}
		}
		Button {
			Layout.alignment: Qt.AlignRight
			visible: step == 'uploading' || step == 'preparing'
			text: 'Abort'
			onClicked: {
				step = 'aborting'
				console.log('ABORT ', stream)
				stream.abort()
			}
		}

	}
}
