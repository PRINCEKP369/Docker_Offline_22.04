Here's how to set up a local APT repository server on Ubuntu 22.04:

## 1. Install Required Tools

```bash
sudo apt update
sudo apt install dpkg-dev apt-utils nginx -y
```

## 2. Create the Repository Directory Structure

```bash
sudo mkdir -p /var/www/apt-repo/pool/main
sudo mkdir -p /var/www/apt-repo/dists/jammy/main/binary-amd64
```

> Adjust `binary-amd64` to `binary-arm64` etc. if needed.

## 3. Copy Your .deb Files

```bash
sudo cp /path/to/your/*.deb /var/www/apt-repo/pool/main/
```

## 4. Generate the Packages Index

```bash
cd /var/www/apt-repo

sudo dpkg-scanpackages --arch amd64 pool/main | sudo tee dists/jammy/main/binary-amd64/Packages
sudo gzip -k -f dists/jammy/main/binary-amd64/Packages
```

## 5. Create the Release File

```bash
cd /var/www/apt-repo/dists/jammy

sudo bash -c 'cat > Release << EOF
Origin: Local Repo
Label: Local
Suite: jammy
Codename: jammy
Date: $(date -Ru)
Architectures: amd64
Components: main
Description: Local APT Repository
EOF'
```

Then generate the checksums and append them:

```bash
sudo bash -c '
do_hash() {
    HASH_NAME=$1
    HASH_CMD=$2
    echo "${HASH_NAME}:"
    for f in $(find -type f -name "Packages*"); do
        echo " $(${HASH_CMD} ${f} | cut -d" " -f1) $(wc -c < ${f}) ${f}"
    done
}
{
    do_hash "MD5Sum" "md5sum"
    do_hash "SHA1"   "sha1sum"
    do_hash "SHA256" "sha256sum"
} >> Release
'
```

## 6. (Optional but Recommended) Sign the Repository with GPG

```bash
# Generate a GPG key if you don't have one
gpg --gen-key

# Sign the Release file
gpg --default-key "Your Name" -abs -o /var/www/apt-repo/dists/jammy/Release.gpg /var/www/apt-repo/dists/jammy/Release
gpg --default-key "Your Name" --clearsign -o /var/www/apt-repo/dists/jammy/InRelease /var/www/apt-repo/dists/jammy/Release

# Export the public key so clients can import it
gpg --export "Your Name" | sudo tee /var/www/apt-repo/repo-key.gpg
```

## 7. Configure Nginx to Serve the Repo

```nginx
# /etc/nginx/sites-available/apt-repo
server {
    listen 80;
    server_name _;   # or your server's hostname/IP

    root /var/www/apt-repo;
    autoindex on;

    location / {
        try_files $uri $uri/ =404;
    }
}
```

```bash
sudo ln -s /etc/nginx/sites-available/apt-repo /etc/nginx/sites-enabled/
sudo nginx -t && sudo systemctl reload nginx
```

## 8. Configure Client PCs

On each client machine on the local network:

```bash
# Add the repo key (if you signed it)
curl http://<SERVER_IP>/repo-key.gpg | sudo gpg --dearmor -o /etc/apt/trusted.gpg.d/local-repo.gpg

# Add the repository source
echo "deb [arch=amd64] http://<SERVER_IP>/ jammy main" | sudo tee /etc/apt/sources.list.d/local-repo.list

# Update and install
sudo apt update
sudo apt install <package-name>
```

If you skipped GPG signing, add `[arch=amd64 trusted=yes]` instead:
```bash
echo "deb [arch=amd64 trusted=yes] http://<SERVER_IP>/ jammy main" | sudo tee /etc/apt/sources.list.d/local-repo.list
```

---

## Updating the Repo Later

Whenever you add new `.deb` files, re-run steps 4 & 5 (and re-sign if applicable), then `sudo apt update` on clients:

```bash
cd /var/www/apt-repo
sudo dpkg-scanpackages --arch amd64 pool/main | sudo tee dists/jammy/main/binary-amd64/Packages
sudo gzip -k -f dists/jammy/main/binary-amd64/Packages
# Regenerate Release file...
```

The `Filename:` field is missing from the Packages entry — that's the problem. apt doesn't know where to fetch the `.deb` from.

Check where the scan is being run from:

```bash
ls /var/www/apt-repo/pool/main/ | grep gcc
```

The `.deb` filename must match exactly. Then regenerate **making sure you are in `/var/www/apt-repo`**:

```bash
cd /var/www/apt-repo
sudo dpkg-scanpackages --arch amd64 pool/main | sudo tee dists/jammy/main/binary-amd64/Packages
sudo gzip -k -f dists/jammy/main/binary-amd64/Packages
```

Then verify the `Filename:` field is now present:

```bash
grep -A 15 "Package: gcc$" /var/www/apt-repo/dists/jammy/main/binary-amd64/Packages
```

You should now see a line like:
```
Filename: pool/main/gcc_4:11.2.0-1ubuntu1_amd64.deb
```

If it's still missing, the `.deb` file might not actually be in `pool/main/`. Share the output of the `ls` command and we'll fix it from there.

The issue is that `dpkg-scanpackages` is being run from the wrong directory or the path is wrong. Try this exact sequence:

```bash
cd /var/www/apt-repo
sudo dpkg-scanpackages pool/main /dev/null | sudo tee dists/jammy/main/binary-amd64/Packages
```

The `/dev/null` is passed as the override file which forces `dpkg-scanpackages` to include the `Filename:` field correctly.

Then verify:
```bash
grep -A 15 "Package: gcc$" dists/jammy/main/binary-amd64/Packages
```

You should see `Filename: pool/main/gcc_...deb` this time.

If still missing, try this alternative using `apt-ftparchive`:

```bash
cd /var/www/apt-repo
sudo apt-ftparchive packages pool/main | sudo tee dists/jammy/main/binary-amd64/Packages
sudo gzip -k -f dists/jammy/main/binary-amd64/Packages
```

`apt-ftparchive` is more reliable at generating correct `Filename:` fields. Then:

```bash
sudo apt update
sudo apt install gcc
```

The problem is that `apt` is finding gcc from the Ubuntu archive first and trying to fetch it from there instead of your local repo. You need to set your local repo as higher priority.

Create a preference file:

```bash
sudo nano /etc/apt/preferences.d/local-repo
```

Paste this:

```
Package: *
Pin: origin 192.168.2.1
Pin-Priority: 1001
```

Save and exit, then:

```bash
sudo apt update
sudo apt install gcc
```

The `Pin-Priority: 1001` forces apt to prefer your local repo over any other source including the Ubuntu archives.

---

**Also double check** that your gcc `.deb` is actually in the repo and indexed:

```bash
# Check it exists in pool
ls /var/www/apt-repo/pool/main/ | grep gcc

# Check it's in the Packages index
grep "Package: gcc" /var/www/apt-repo/dists/jammy/main/binary-amd64/Packages
```

If it's missing from the Packages index, re-run the scan:

```bash
cd /var/www/apt-repo
sudo dpkg-scanpackages --arch amd64 pool/main | sudo tee dists/jammy/main/binary-amd64/Packages
sudo gzip -k -f dists/jammy/main/binary-amd64/Packages
```

Then regenerate the Release file and do `sudo apt update` again.
