import QtTest 1.0
import org.kde.bluezqt.fakebluez 1.0

TestCase {
    name: "Device"
    property var manager;
    property var device1props;
    property var device2props;

    TestUtils {
        id: utils
    }

    function initTestCase()
    {
        FakeBluez.start();
        FakeBluez.runTest("bluez-standard");

        // Create adapters
        var adapter1path = "/org/bluez/hci0";
        var adapter1props = {
            Path: adapter1path,
            Address: "1C:E5:C3:BC:94:7E",
            Name: "TestAdapter",
            _toDBusObjectPath: [ "Path" ]
        }
        FakeBluez.runAction("devicemanager", "create-adapter", adapter1props);

        var adapter2path = "/org/bluez/hci1";
        var adapter2props = {
            Path: adapter2path,
            Address: "2E:3A:C3:BC:85:7C",
            Name: "TestAdapter2",
            _toDBusObjectPath: [ "Path" ]
        }
        FakeBluez.runAction("devicemanager", "create-adapter", adapter2props);

        // Create devices
        device1props = {
            Path: "/org/bluez/hci0/dev_40_79_6A_0C_39_75",
            Adapter: adapter1path,
            Address: "40:79:6A:0C:39:75",
            Name: "TestDevice",
            Alias: "TestAlias",
            Icon: "phone",
            Class: 101,
            Appearance:  25,
            UUIDs: [ "00001200-0000-1000-8000-00805f9b34fb" ],
            Paired: false,
            Connected: false,
            Trusted: false,
            Blocked: false,
            LegacyPairing: false,
            RSSI: 20,
            Modalias: "bluetooth:v001Dp1200d1236",
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device1props);

        device2props = {
            Path: "/org/bluez/hci1/dev_50_79_6A_0C_39_75",
            Adapter: adapter2path,
            Address: "50:79:6A:0C:39:75",
            Name: "TestDevice2",
            Alias: "TestAlias2",
            Icon: "joypad",
            Class: 201,
            Appearance:  32,
            UUIDs: [ "00001124-0000-1000-8000-00805f9b34fb" ],
            Paired: true,
            Connected: false,
            Trusted: true,
            Blocked: false,
            LegacyPairing: true,
            RSSI: -15,
            Modalias: "bluetooth:v001Dp1200d1136",
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device2props);

        manager = utils.createManager(this);

        var initResult = utils.initManager(manager);
        verify(initResult, "initFinished", "init-manager");
        compare(manager.adapters.length, 2, "adapters-length");
        compare(manager.devices.length, 2, "devices-length");
    }

    function cleanupTestCase()
    {
        FakeBluez.stop();
        manager.destroy();
    }

    function compareProperties(device, props)
    {
        compare(device.ubi, props.Path, "ubi");
        compare(device.address, props.Address, "address");
        compare(device.name, props.Alias, "name");
        compare(device.remoteName, props.Name, "remoteName");
        compare(device.icon, props.Icon, "icon");
        compare(device.deviceClass, props.Class, "deviceClass");
        compare(device.appearance, props.Appearance, "appearance");
        compare(device.paired, props.Paired, "paired");
        compare(device.connected, props.Connected, "connected");
        compare(device.trusted, props.Trusted, "trusted");
        compare(device.blocked, props.Blocked, "blocked");
        compare(device.legacyPairing, props.LegacyPairing, "legacyPairing");
        compare(device.rssi, props.RSSI, "rssi");
        compare(device.modalias, props.Modalias, "modalias");

        compare(manager.adapterForDevice(device).ubi, props.Adapter, "adapter");

        for (var i = 0; i < device.uuids.length; ++i) {
            compare(device.uuids[i], props.UUIDs[i].toUpperCase(), "uuids" + i);
        }
    }

    function test_getProperties()
    {
        var device1 = manager.deviceForUbi(device1props.Path);
        var device2 = manager.deviceForUbi(device2props.Path);
        compareProperties(device1, device1props);
        compareProperties(device2, device2props);
    }

    SignalSpy {
        id: nameChangedSpy
        signalName: "nameChanged"
    }

    function test_setName()
    {
        for (var i = 0; i < manager.devices.length; ++i) {
            var device = manager.devices[i];
            var originalValue = device.name;
            var value = originalValue + "tst_name";

            nameChangedSpy.clear();
            nameChangedSpy.target = device;

            device.name = value;

            tryCompare(nameChangedSpy, "count", 1);
            compare(device.name, value);

            device.name = originalValue;
        }
    }

    SignalSpy {
        id: trustedChangedSpy
        signalName: "trustedChanged"
    }

    function test_setTrusted()
    {
        for (var i = 0; i < manager.devices.length; ++i) {
            var device = manager.devices[i];
            var originalValue = device.trusted;
            var value = !originalValue;

            trustedChangedSpy.clear();
            trustedChangedSpy.target = device;

            device.trusted = value;

            tryCompare(trustedChangedSpy, "count", 1);
            compare(device.trusted, value);

            device.trusted = originalValue;
        }
    }

    SignalSpy {
        id: managerDeviceRemovedSpy
        signalName: "deviceRemoved"
    }

    SignalSpy {
        id: adapterDeviceRemovedSpy
        signalName: "deviceRemoved"
    }

    SignalSpy {
        id: deviceDeviceRemovedSpy
        signalName: "deviceRemoved"
    }

    // za prefix to force test order - last
    function test_za_DeviceRemoved()
    {
        while (manager.devices.length > 0) {
            var device = manager.devices[0];

            managerDeviceRemovedSpy.clear();
            managerDeviceRemovedSpy.target = manager;
            adapterDeviceRemovedSpy.clear();
            adapterDeviceRemovedSpy.target = manager.adapterForDevice(device);
            deviceDeviceRemovedSpy.clear();
            deviceDeviceRemovedSpy.target = device;

            var props = {
                Path: device.ubi,
                _toDBusObjectPath: [ "Path" ]
            }
            FakeBluez.runAction("devicemanager", "remove-device", props);

            tryCompare(managerDeviceRemovedSpy, "count", 1);
            tryCompare(adapterDeviceRemovedSpy, "count", 1);
            tryCompare(deviceDeviceRemovedSpy, "count", 1);
        }
    }
}
