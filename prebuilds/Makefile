%.framework: %.framework.tar
	tar -xf $< --directory $(dir $<)

%.framework:
	rm -rf $@
	mkdir -p $(dir $@)
	cp -a $< $@
	lipo -create -output $@/$(basename $(notdir $@)) $(addsuffix /$(basename $(notdir $@)),$^)

%.xcframework:
	rm -rf $@
	xcodebuild -create-xcframework -output $@ $(addprefix -framework , $^)

dotnet%:
	rm -rf $@
	mkdir -p $@
	cp -a dotnet-* $@
	@echo "Copied .NET files to $@:"
	ls -all $@

%: %-release.aar
	rm -rf $@
	mkdir -p $@
	unzip -q $< -d $@

# Default target
all: ios/BareKit.xcframework android/bare-kit dotnet

# iOS XCFramework
ios/BareKit.xcframework: ios/BareKit.framework ios-simulator/BareKit.framework

ios/BareKit.framework: ios-arm64/BareKit.framework

ios-simulator/BareKit.framework: ios-arm64-simulator/BareKit.framework ios-x64-simulator/BareKit.framework

# Android AAR unpacking
android/bare-kit: android/bare-kit-release.aar
	mkdir -p $@
	unzip -q $< -d $@

# .NET
dotnet: dotnet/runtime/windows-x64 dotnet/runtime/darwin-x64 dotnet/runtime/linux-x64

dotnet: dotnet/runtime/windows-x64 dotnet/runtime/darwin-x64 dotnet/runtime/linux-x64

dotnet/runtime/windows-x64: prebuilds/dotnet-windows-x64
	rm -rf $@
	mkdir -p $@
	cp -a $</* $@
	@echo "Copied .NET Windows files to $@:"
	ls -all $@

dotnet/runtime/darwin-x64: prebuilds/dotnet-darwin-x64
	rm -rf $@
	mkdir -p $@
	cp -a $</* $@
	@echo "Copied .NET macOS files to $@:"
	ls -all $@

dotnet/runtime/linux-x64: prebuilds/dotnet-linux-x64
	rm -rf $@
	mkdir -p $@
	cp -a $</* $@
	@echo "Copied .NET Linux files to $@:"
	ls -all $@

# Clean target
clean:
	rm -rf ios android dotnet
