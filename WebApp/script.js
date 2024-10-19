// Function to check if the user is authenticated
function isAuthenticated() {
    return localStorage.getItem("authenticated") === "true";
}

// Function to set the authentication status in localStorage
function setAuthenticationStatus(status) {
    localStorage.setItem("authenticated", status);
}

// Function to set data in localStorage
function setData(data) {
    localStorage.setItem("data", JSON.stringify(data));
}

// Function to get data from localStorage or initialize with default values
function getData() {
    var storedData = localStorage.getItem("data");
    if (!storedData) {
        var defaultData = ["", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""];
        setData(defaultData);
        return defaultData;
    }
    return JSON.parse(storedData);
}

// Function to update data in localStorage based on current page elements
function updateData() {
    var data = ["", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""];
    for (var i = 1; i <= 5; i++) {
        data[i - 1] = document.getElementById('speed' + i).textContent || document.getElementById('speed' + i).innerText;
        data[i + 4] = document.getElementById('status' + i).textContent || document.getElementById('status' + i).innerText;
		var pClass = document.getElementById('pdc' + i).textContent || document.getElementById('pdc' + i).innerText;
		if(pClass !== "No") {
			if(parseInt(pClass, 10) <= 4) {
				data[i + 9] = "ieee3at";
				data[i + 14] = "enable";
			} else {
				data[i + 9] = "ieee3bt";
				data[i + 14] = "enable";
			}
		} else {	
			data[i + 9] = "ieee3at";
			data[i + 14] = "disable";
		}
			
    }
    setData(data);
}

// Variable to store the current page URL
var currentPage = window.location.href;

// Function to highlight the active link in the menu bar
function menuBar() {
    var links = document.querySelectorAll('.menu-bar a');
    links.forEach(function (link) {
        if (link.href === currentPage) {
            link.classList.add('active');
        }
    });
}

// Function to update power circle color based on linkStatus value
function updatePowerCircle(linkStatus, pdClass, powerCircleId) {
    var powerCircle = document.getElementById(powerCircleId);
	powerCircle.classList.toggle("green", linkStatus === "up" && pdClass !== "No");
	powerCircle.classList.toggle("red", linkStatus !== "up" && pdClass === "No");
	powerCircle.classList.toggle("amber", linkStatus === "up" && pdClass === "No");

}

// Function to update page content if needed
function updatePageIfNeeded() {
    if (currentPage.includes("switch_status.shtml")) {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
				var returnedPage = this.responseText;
				var tempDiv = document.createElement("div");
				tempDiv.innerHTML = returnedPage;
				var pageTitle = tempDiv.querySelector("#sp").innerHTML;
				if (pageTitle === "Networking Switch Status") {
					document.body.innerHTML = this.responseText;
				} else if (pageTitle === "Switch login") {
				}
				for (var i = 1; i <= 5; i++) {
					var linkStatusElement = document.getElementById("linkStatus" + i);
					var linkStatus = linkStatusElement.textContent || linkStatusElement.innerText;
					var pdClassElement = document.getElementById("pdc" + i);
					var pdClass = pdClassElement.textContent || pdClassElement.innerText;
					updatePowerCircle(linkStatus, pdClass, "powerCircle" + i);
				}
				menuBar();
				updateData();
            }
        };
        xhttp.open("GET", "update_page.cgi", true);
        xhttp.send();
    }
}

// Function to send a login request
function sendLoginRequest(value1, value2) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            var returnedPage = this.responseText;
            var tempDiv = document.createElement("div");
            tempDiv.innerHTML = returnedPage;
            var pageTitle = tempDiv.querySelector("#sp").innerHTML;
            if (pageTitle === "Networking Switch Status") {
                window.location.href = "switch_status.shtml";
                setAuthenticationStatus(true);
                updateData();
            } else if (pageTitle === "Switch login") {
                window.location.href = "index.html";
                setAuthenticationStatus(false);
            }
        }
    };

    var loginURL = "login.cgi?username=" + encodeURIComponent(value1) + "&password=" + encodeURIComponent(value2);
    xhttp.open("GET", loginURL, true);
    xhttp.send();
}

// Function to send changes to the server
function sendChanges() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            window.location.href = "switch_status.shtml";
        }
    };
    var queryParams = '';

    for (var portNumber = 1; portNumber <= 5; portNumber++) {
        var speed = document.getElementById('sspeed' + portNumber).value;
        var status = document.getElementById('sstatus' + portNumber).value;
        var pdconfig = document.getElementById('pdconfig' + portNumber).value;
        var pdenable = document.getElementById('pdenable' + portNumber).value;

        queryParams += 'port' + portNumber + '=' + speed +
            ',' + status +
            ',' + pdconfig +
            ',' + pdenable;

        if (portNumber < 5) {
            queryParams += '&';
        }
    }

    var url = '/setting.cgi?' + queryParams;
    xhttp.open("GET", url, true);
    xhttp.send();
    console.log(queryParams);
}

// Function to update the setting page with current data
function updateSettingPage() {
    var data = getData();
    for (var i = 1; i <= 5; i++) {
        document.getElementById('sspeed' + i).value = data[i - 1];
        document.getElementById('sstatus' + i).value = data[i + 4];
		document.getElementById('pdconfig' + i).value = data[i + 9];
        document.getElementById('pdenable' + i).value = data[i + 14];
    }
}

// Function to check updates
function checkForUpdate() {
    const apiUrl = 'https://api.github.com/repos/Ghost899x/PoE_Switch/releases/latest';

    fetch(apiUrl)
        .then(response => response.json())
        .then(data => {

            const latestVersion = data.tag_name;
			
			var xhttp = new XMLHttpRequest();
			xhttp.onreadystatechange = function() {
				if(xhttp.readyState == 4 && xhttp.status == 200) {
					var returnedPage = this.responseText;
					var tempDiv = document.createElement("div");
					tempDiv.innerHTML = returnedPage;
					var pageTitle = tempDiv.querySelector("#up").innerHTML;
					if (pageTitle === "Networking Switch Update") {
						const shouldDownload = confirm(`A new version (${latestVersion}) is available. Do you want to download it?`);					
						if (shouldDownload) {
							downloadUpdate(data.assets[0].browser_download_url);
						} else {
							alert('Update declined.');
						}
					} else{
						alert('You are using the latest version.');
					}
				}
			}			
			var updateRequest = "updateRequest.cgi?latestVersion=" + latestVersion;
			xhttp.open("GET", updateRequest, true);
			xhttp.send();
        })
        .catch(error => console.error('Error checking for update:', error));
}

// Function to download the updates
function downloadUpdate(updateUrl) {
    // Create a link element
    const link = document.createElement('a');
    link.href = updateUrl;
    link.download = 'updated_app.zip'; 

    // Append the link to the body
    document.body.appendChild(link);

    // Trigger the download
    link.click();

    // Remove the link from the DOM
    document.body.removeChild(link);
}

// Function to upload the firmware
function uploadFile() {
    const fileInput = document.getElementById('fileInput');
	
    // Check if a file is selected
    if (fileInput.files.length > 0) {
        const file = fileInput.files[0];
        const reader = new FileReader();

	document.getElementById('uploadButton').style.display = 'none';
    document.getElementById('uploadProgress').style.display = 'block';
	document.getElementById('uploadStatus').style.display = 'block';

        // Set up event handler for when the file is loaded
        reader.onload = function (e) {
            const fileContents = e.target.result;
            const fileSizeBytes = file.size;

            // Create a buffer array with slices of 1 kilobyte each
            const buffer = [];
            for (let i = 0; i < fileSizeBytes; i += 1024) {
                const slice = fileContents.slice(i, i + 1024);
                buffer.push(slice);
            }
			updateState("starting firmware upload", (fileSizeBytes / 1024).toFixed(2), buffer.length, function (status) {
				if (status === "success") {
					document.getElementById('uploadStatusText').innerText = "Starting";
					uploadChunks(buffer, fileSizeBytes);
				} else {
					document.getElementById('uploadStatusText').innerText = "Upload Failed";
				}
			});
		};

        // Read the file as binary
        reader.readAsArrayBuffer(file);
    } else {
        console.log("No file selected.");
    }
}

// Function to upload chunks of firmware
function uploadChunks(buffer, fileSizeBytes) {
    function sendNextChunk(index) {
        if (index < buffer.length) {
            // Send the current chunk
            sendChunk(buffer[index], function (status) {
                if (status === "success") {
					document.getElementById('uploadProgress').value = ((index + 1) / (buffer.length / 100));
					document.getElementById('uploadStatusText').innerText = "Uploading";
                    setTimeout(function () {
                        sendNextChunk(index + 1);
                    }, 10);
                } else {
                    document.getElementById('uploadStatusText').innerText = "Upload Failed";
                }
            });
        } else {
            // When all chunks are sent, update the state for completion
            updateState("firmware upload completed", (fileSizeBytes / 1024).toFixed(2), buffer.length, function (status) {
                if (status === "success") {
					document.getElementById('uploadStatusText').innerText = "Successful";
					const resetButton = document.getElementById('resetButton');
					resetButton.style.display = 'inline-block';
                } else {
                    document.getElementById('uploadStatusText').innerText = "Upload Failed";
                }
            });
        }
    }

    // Start sending the chunks with an initial index of 0
    sendNextChunk(0);
}

// Function to send the chunk to controller
function sendChunk(data, callback)
{
		var http = new XMLHttpRequest();
		var url = 'upload.cgi';
		var params = new Uint8Array(data);

		http.open('POST', url, true);

		//Send the proper header information along with the request
		http.setRequestHeader('Content-type', 'application/octet-stream');
		http.setRequestHeader('X-CRC32', calculateCRC32(params)); 

		http.onreadystatechange = function() {
			if(http.readyState == 4 && http.status == 200) {
				var returnedPage = this.responseText;
				var tempDiv = document.createElement("div");
				tempDiv.innerHTML = returnedPage;
				var pageTitle = tempDiv.querySelector("#up").innerHTML;
				if (pageTitle === "Networking Switch Update") {
					callback("success");
				}
				else{
					callback("failed");
				}
			}
		}
		http.send(params);
}

// Function to handle the upload process
function updateState(state, size, chunks, callback)
{
		var http = new XMLHttpRequest();
		var url = 'upload.cgi';
		var param = chunks;	
		var header = state + "," + chunks + "," + size;
		console.log(header);
		http.open('POST', url, true);

		//Send the proper header information along with the request
		http.setRequestHeader('Content-type', 'application/octet-stream');
		http.setRequestHeader('State', header); 
		
		http.onreadystatechange = function() {
			if(http.readyState == 4 && http.status == 200) {
				var returnedPage = this.responseText;
				var tempDiv = document.createElement("div");
				tempDiv.innerHTML = returnedPage;
				var pageTitle = tempDiv.querySelector("#up").innerHTML;
				if (pageTitle === "Networking Switch Update") {
					callback("success");
				}
				else{
					callback("failed");
				}
			}
		}
		http.send(param);
}

// Function to reset the controller 
function resetUpload() {
    // Hide the progress bar and show the upload button
    document.getElementById('uploadProgress').style.display = 'none';
    document.getElementById('uploadStatus').style.display = 'none';

    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            // Wait for 1 second (1000 milliseconds) before redirecting
            setTimeout(function () {
                window.location.href = 'index.html';
				setAuthenticationStatus(false);
            }, 1000);
        }
    };

    var reset = "reset.cgi?update=" + 1;
    xhttp.open("GET", reset, true);
    xhttp.send();
}

// Function to calculate the crc
function calculateCRC32(uint8Array) {
    // Precompute CRC32 table
    var crc32Table = (function() {
        var table = new Uint32Array(256);
        for (var i = 0; i < 256; i++) {
            var crc = i;
            for (var j = 0; j < 8; j++) {
                crc = (crc & 1 ? 0xEDB88320 : 0) ^ (crc >>> 1);
            }
            table[i] = crc >>> 0;
        }
        return table;
    })();

    var crc = 0xFFFFFFFF;

    for (var i = 0; i < uint8Array.length; i++) {
        crc = (crc >>> 8) ^ crc32Table[(crc ^ uint8Array[i]) & 0xFF];
    }

    // XOR with 0xFFFFFFFF to get the final CRC value
    return (crc ^ 0xFFFFFFFF) >>> 0;
}

// Function to update the displayed file name or default text
function displayFileName() {
    const fileInput = document.getElementById('fileInput');
    const fileNameSpan = document.getElementById('fileName');
    if(fileInput)
	{
		if (fileInput.files.length > 0) {
			fileNameSpan.textContent = fileInput.files[0].name;
		} else {
			fileNameSpan.textContent = 'Choose File'; // Set default text if no file is chosen
		}
	}
}

// Call the function initially to set the default text
displayFileName();

function toggleVisibility(parameter, object1, object2, object3, object4  ) {
	var mode = parameter.value;
	if(object4){
		object1.style.display = mode === '0' ? 'none' : 'block';
		object2.style.display = mode === '0' ? 'none' : 'block';
		object3.style.display = mode === '0' ? 'none' : 'block';
		object4.style.display = mode === '0' ? 'none' : 'block';
		
		var address = object4.querySelector('label');
		var input = object4.querySelector('input');
		var option = object1.querySelector('select');
		
		if(mode === '1'){
            if (address) {
                address.textContent = 'MAC Address';
				input.maxLength = '17';
            }
			option.value = '1';
		}else if(mode === '2'){
            if (address) {
                address.textContent = 'IP Address';
				input.maxLength = '15';
            }
			option.value = '20';
		}else {
			object3.style.display = 'none';
			object2.style.display = 'none' ;
		}
		var ipv4Options = object1.querySelectorAll('option[data-mode="ipv4"]');
		ipv4Options.forEach(function(option) {
			option.style.display = mode === '2' ? 'block' : 'none';
		});
		var macOptions = object1.querySelectorAll('option[data-mode="mac"]');
		macOptions.forEach(function(option) {
			option.style.display = mode === '1' ? 'block' : 'none';
		});
	} else if(mode === '2' && object1.value === '10') {
		object3.style.display = 'block';
		object2.style.display = 'none' ;
	} else if(mode === '1' && object1.value !== '1') {
		object1.style.display = mode === '0' ? 'none' : 'block';
		object3.style.display = 'none';
		object2.style.display = 'block' ;
	} else {
		object3.style.display = 'none';
		object2.style.display = 'none' ;
	}
}

function toggleValue(checkbox) {
	checkbox.value = checkbox.checked ? '1' : '0';
}
function submitACL() {
	var queryParams = 'port='+ document.getElementById('portNumber').value + ',' +
	'auth='+ document.getElementById('aclmode').value + ',' +
	'service=' + document.getElementById('toggleCheckbox').value + ',';
	var entryCount = 0;
    for (var entry = 1; entry <= 2; entry++) {
		var mode = document.getElementById('mode' + entry).value;
		console.log(mode);
		if(mode !== '0') {		
			entryCount++;
			var compare = document.getElementById('compare' + entry).value;
			var sd = document.getElementById('s/d' + entry).value;
			var permission = document.getElementById('permission' + entry).value;
			var address = document.getElementById('address' + entry).value;
			var type = document.getElementById('type' + entry).value;
			var mask = document.getElementById('mask' + entry).value;
			if(address === '') {
				alert("Missing Parameters");
				return 0;
			}
			
			queryParams += 'entry' + entry + ',' +
			'mode=' + mode + ',' + 'compare=' + compare + ',' + 
			'sd=' + sd + ',' + 'permission=' + permission + ',' ;
			
			switch(compare){
			case '1':
				queryParams += 'mac=' + address + '&';
				break;
			case '2':
				queryParams += 'mac=' + address + ',';
				queryParams += 'type=' + type + '&';
				break;
			case '10':
				if(mask === '') {
					alert("Missing Parameters");
					return 0;
				}
				queryParams += 'ip=' + address + ',';
				queryParams += 'mask=' + mask + '&';
				break;
			case '20':
				queryParams += 'ip=' + address + '&';
				break;
			default:
				queryParams += 'mac=' + address + ',';
				queryParams += 'type=' + type + '&';
				break;
			}
		}
    }
	console.log(queryParams);
	if(entryCount > 0) { 
	    var http = new XMLHttpRequest();
		var url = 'acl.cgi';

		http.open('POST', url, true);

		http.setRequestHeader('Content-type', 'acl-table/entries');
		http.setRequestHeader('Entries', entryCount); 

		http.onreadystatechange = function() {
			if(http.readyState == 4 && http.status == 200) {
				var returnedPage = this.responseText;
				var tempDiv = document.createElement("div");
				tempDiv.innerHTML = returnedPage;
				var pageTitle = tempDiv.querySelector("#up").innerHTML;
				if (pageTitle === "Access Control List") {
					console.log("ok");
				}
				else{
					console.log("error");
				}
			}
		}
		http.send(queryParams);
	}
}

function setVLAN()
{
	var queryParams = 'port='+ document.getElementById('portNumber').value + ',' +
	'service=' + document.getElementById('toggleCheckbox').value + ',';
	
	for (var port = 1; port <= 6; port++)
	{
		queryParams += "port" + port + "=" + document.getElementById('p' + port).value;
		if(port < 6)
		{
			queryParams += ",";
		}	 
	}
	console.log(queryParams);
	
		var http = new XMLHttpRequest();
		var url = 'vlan.cgi';

		http.open('POST', url, true);

		http.setRequestHeader('Content-type', 'vlan/groups');

		http.onreadystatechange = function() {
			if(http.readyState == 4 && http.status == 200) {
				var returnedPage = this.responseText;
				var tempDiv = document.createElement("div");
				tempDiv.innerHTML = returnedPage;
				var pageTitle = tempDiv.querySelector("#up").innerHTML;
				if (pageTitle === "VLAN") {
					console.log("ok");
				}
				else{
					console.log("error");
				}
			}
		}
		http.send(queryParams);
}

// Function to handle page loading
document.addEventListener("DOMContentLoaded", function () {
    if (isAuthenticated() && currentPage.includes("index.html")) {
        setAuthenticationStatus(false);
    }
    if (!isAuthenticated() && !currentPage.includes("index.html")) {
        window.location.href = "index.html";
    }
    updatePageIfNeeded();

    setInterval(updatePageIfNeeded, 5000);
	menuBar();

    var loginButton = document.getElementById("loginButton");
    if (loginButton) {
        loginButton.addEventListener("click", function () {
            var username = document.getElementById("username").value;
            var password = document.getElementById("password").value;
            sendLoginRequest(username, password);
        });
    }

    var applyChanges = document.getElementById("applyChanges");
    if (applyChanges) {
        applyChanges.addEventListener('click', function () {
            sendChanges();
        });
    }
	
	var submitChanges = document.getElementById("submitChanges");
    if (submitChanges) {
        submitChanges.addEventListener('click', function () {
            submitACL();
        });
    }
	
	var setButton = document.getElementById("setButton");
    if (setButton) {
        setButton.addEventListener('click', function () {
            setVLAN();
        });
    }

    var pdenable1 = document.getElementById("pdenable1");
    if (pdenable1) {
        updateSettingPage();
		menuBar();
    }
	var fileInput = document.getElementById("fileInput");
    if (fileInput) {
       fileInput.addEventListener('change', displayFileName);
    }

});
