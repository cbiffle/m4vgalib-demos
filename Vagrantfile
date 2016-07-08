# vi: set ft=ruby :

$stlink_vid = '0x0483'
$stlink_pid = '0x3748'

Vagrant.configure(2) do |config|
	config.vm.box = 'ubuntu/trusty64'

	config.vm.provision "shell", privileged: false, path: "vm-bootstrap.sh"

	config.vm.provider "virtualbox" do |v|
		v.memory = 1024
                v.customize ['modifyvm', :id, '--usb', 'on']
                v.customize [
                  'usbfilter', 'add', '0', '--target', :id,
                                           '--name', 'STLink',
                                           '--vendorid', $stlink_vid,
                                           '--productid', $stlink_pid]
	end
end
