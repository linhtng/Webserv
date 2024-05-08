function sendHeadRequest() {
	var url = 'http://localhost:10001/';
	fetch(url, {
			method: 'HEAD',
			headers: {
					'Host': 'localhost:10001',
			},
	})
	.then(response => {
			if (!response.ok) {
					throw new Error('HEAD request unsuccessful, status code: ' + response.status + ', ' + response.statusText);
			}
			showMessage('HEAD request successful, status code: ' + response.status, false, 'headMessage');
	})
	.catch(error => {
			showMessage(error.message, true, 'headMessage');
	});
}

function sendDeleteRequest() {
	var url = 'http://localhost:10001/upload/number.txt';
	fetch(url, {
			method: 'DELETE',
			headers: {
					'Host': 'localhost:10001',
			},
	})
	.then(response => {
			if (!response.ok) {
					throw new Error('File not deleted, status code: ' + response.status + ', ' + response.statusText);
			}
			showMessage('File deleted successfully, status code: ' + response.status, false, 'deleteMessage');
	})
	.catch(error => {
			showMessage(error.message, true, 'deleteMessage');
	});
}

function showMessage(message, isError, elementId) {
	var messageDiv = document.getElementById(elementId);
	messageDiv.textContent = message;
	messageDiv.style.display = 'block';
	messageDiv.className = isError ? 'message error' : 'message success';
}