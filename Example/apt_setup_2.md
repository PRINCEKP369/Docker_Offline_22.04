Setting up a local APT repository server for Ubuntu 22.04 (Jammy) using .deb files is a solid approach for offline or air-gapped environments—very relevant to your workflow.

I’ll walk you through a clean, production-style setup using dpkg-scanpackages + HTTP (Apache). This is the most reliable and simple method.

⸻

🧱 Architecture Overview
	•	One machine = APT server
	•	Other machines = clients (LAN connected)
	•	Transport = HTTP (recommended over file:// for multiple systems)

⸻

⚙️ Step 1: Install Required Tools (on server)

sudo apt update
sudo apt install dpkg-dev apache2 -y

	•	dpkg-dev → generates package index
	•	apache2 → serves repo over HTTP

⸻

📁 Step 2: Create Repository Structure

sudo mkdir -p /var/www/html/ubuntu-repo
cd /var/www/html/ubuntu-repo

Copy all your .deb files:

sudo cp /path/to/your/debs/*.deb .


⸻

📦 Step 3: Generate Package Index

dpkg-scanpackages . /dev/null | gzip -9c > Packages.gz

This creates:
	•	Packages.gz → metadata used by APT

👉 If you want uncompressed also (optional):

dpkg-scanpackages . /dev/null > Packages


⸻

🌐 Step 4: Start Apache Server

sudo systemctl enable apache2
sudo systemctl start apache2

Test in browser (client or server):

http://<server-ip>/ubuntu-repo/

You should see .deb files listed.

⸻

💻 Step 5: Configure Client Machines

On each client system:

Add source

sudo nano /etc/apt/sources.list.d/localrepo.list

Add:

deb [trusted=yes] http://<server-ip>/ubuntu-repo ./

trusted=yes skips GPG signing (fine for internal use)

⸻

🔄 Step 6: Update APT

sudo apt update

You should see it fetching:

http://<server-ip>/ubuntu-repo Packages


⸻

📥 Step 7: Install Packages

sudo apt install <package-name>


⸻

⚠️ Important Notes (Critical)

1. Dependencies

APT does NOT auto-resolve missing dependencies unless:
	•	You include ALL required .deb files

👉 Use this on an online system to gather dependencies:

apt download <package>
apt-rdepends <package>

Or better:

apt-get download $(apt-rdepends <package> | grep -v "^ ")


⸻

2. Flat Repo vs Structured Repo

What we created is a flat repository:

ubuntu-repo/
 ├── package1.deb
 ├── package2.deb
 └── Packages.gz

APT supports this via ./ in sources.

⸻

🚀 Optional: Better Structured Repo (Advanced)

If you want Ubuntu-like structure:

dists/jammy/main/binary-amd64/

Use tools like:
	•	apt-ftparchive (manual)
	•	reprepro (recommended for large repos)

⸻

🔐 Optional: Add GPG Signing (Production)

For secure environments:
	•	Generate GPG key
	•	Sign Release file
	•	Remove [trusted=yes]

⸻

🧪 Quick Test Workflow

On client:

sudo apt update
apt-cache policy <package-name>

You should see your repo listed.

⸻

🧠 Practical Tip (Your Use Case)

Since you’re working in offline + Docker + embedded environments, I recommend:
	•	Maintain repo on external HDD / NAS
	•	Periodically sync using:

rsync -av repo/ server:/var/www/html/ubuntu-repo/


	•	Version your repo (e.g., repo-v1, repo-v2)

⸻

🔁 Alternative (No HTTP)

If only one machine:

deb [trusted=yes] file:/path/to/repo ./

But for multiple PCs → HTTP is cleaner.

⸻

✅ Summary

Step	Action
1	Install dpkg-dev, apache2
2	Place .deb files in /var/www/html/ubuntu-repo
3	Generate Packages.gz
4	Serve via Apache
5	Add repo in clients
6	apt update + install


⸻

If you want, I can help you next with:
	•	🔧 Automating repo updates
	•	📦 Building a full offline Ubuntu mirror
	•	🐳 Using this repo inside Docker builds
	•	⚡ Dependency harvesting script for offline systems

Just tell me your target workflow.
