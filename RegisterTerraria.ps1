$path = "C:\Users\maja\Downloads\terraria_xb1_dump\terraria_xb1_dump\Mount\AppxManifest.xml"

$xml = '<?xml version="1.0" encoding="utf-8"?>
<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
         xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest"
         xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
         xmlns:rescap="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities"
         IgnorableNamespaces="uap mp rescap">
  <Identity Name="505780Re-Logic.Terraria"
            Publisher="CN=Re-Logic"
            Version="1.0.0.0" />
  <mp:PhoneIdentity PhoneProductId="00000000-0000-0000-0000-000000000000"
                    PhonePublisherId="00000000-0000-0000-0000-000000000000"/>
  <Properties>
    <DisplayName>Terraria</DisplayName>
    <PublisherDisplayName>Re-Logic</PublisherDisplayName>
    <Logo>Assets\StoreLogo.png</Logo>
  </Properties>
  <Dependencies>
    <TargetDeviceFamily Name="Windows.Desktop"
                        MinVersion="10.0.17763.0"
                        MaxVersionTested="10.0.19041.0"/>
  </Dependencies>
  <Resources>
    <Resource Language="en-us"/>
  </Resources>
  <Applications>
    <Application Id="App" Executable="Terraria.exe"
                 EntryPoint="Windows.FullTrustApplication">
      <uap:VisualElements DisplayName="Terraria"
                          Description="Terraria"
                          BackgroundColor="transparent"
                          Square150x150Logo="Assets\Logo.png"
                          Square44x44Logo="Assets\SmallLogo.png">
      </uap:VisualElements>
    </Application>
  </Applications>
  <Capabilities>
    <rescap:Capability Name="runFullTrust"/>
  </Capabilities>
</Package>'

Set-Content -Path $path -Value $xml -Encoding UTF8
Write-Host "AppxManifest.xml written!" -ForegroundColor Green

Add-AppxPackage -Path $path -Register
Write-Host "Done!" -ForegroundColor Green
