import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.1

ApplicationWindow {
	visible: true
	width: 640
	height: 480
	minimumWidth: 480
	minimumHeight: 240
	title: qsTr('QuickStreams - File Upload Example')

	// Filesystem simulator component
	FilesystemMock {
		id: fs
		minNetworkLatency: 1000
		maxNetworkLatency: 2000
	}

	// Environment variables
	property int chunkSize: 10

	ListModel {
		id: uploadList
	}

	function createNewUpload(fileSize) {
		// Create a new asynchronous file upload stream
		var uploadStream = fs.uploadFile({
			fileSize: fileSize,
			chunkSize: 2
		})

		uploadList.append({
			identifier: '',
			size: fileSize,
			progress: 0,
			step: 'preparing',
			stream: uploadStream,
			failureReason: ''
		})

		var listIndex = uploadList.count - 1

		// When the upload stream's closed the upload is considered finished
		uploadStream.attach(function(stream, fileId) {
			console.log('file successfuly uploaded:', fileId)
			uploadList.setProperty(listIndex, 'step', 'finished')
		})
		// Handle unexpected errors on the chain
		.failure(function(stream, error) {
			console.log('ERROR:', error)
			uploadList.setProperty(listIndex, 'step', 'failed')
			uploadList.setProperty(listIndex, 'failureReason', error)
		})

		// Handle file allocation success event
		uploadStream.event('allocated', function(data) {
			console.log('file successfuly allocated (', data.id, ')')
			uploadList.setProperty(listIndex, 'step', 'uploading')
			uploadList.setProperty(listIndex, 'identifier', data.id)
		})

		// Update progress on "chunk_uploaded"
		uploadStream.event('chunk_uploaded', function(data) {
			console.log('chunk uploaded (', data.progress, '/', fileSize, ')')
			uploadList.setProperty(listIndex, 'progress', data.progress)
		})

		uploadStream.event(
			'cleanup_after_failure',
			function() {
				console.log('failed, cleaning up...')
				uploadList.setProperty(listIndex, 'step', 'failed')
			}
		)

		uploadStream.event(
			'cleanup_after_abortion',
			function() {
				console.log('aborted, cleaning up...')
				uploadList.setProperty(listIndex, 'step', 'aborting')
			}
		)

		// Update UI when abortion is completed
		uploadStream.abortion(function(stream) {
			console.log('upload was aborted')
			uploadList.setProperty(listIndex, 'step', 'aborted')
			reset()
		})

		// This console output visualizes the end of the declaration tick
		console.log('--------- MAIN CHAIN DECLARED ---------')
	}

	header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            ToolButton {
                text: qsTr('New Upload')
                Layout.fillWidth: false
                Layout.preferredWidth: 96
                Layout.alignment: Qt.AlignLeft
                onClicked: createNewUpload(10)
            }
        }
    }

	ListView {
		model: uploadList
		anchors.fill: parent
		anchors.topMargin: 8
		delegate: ListDelegate {
			width: parent.width
			height: 48
		}
	}
}
