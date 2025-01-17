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

%.%bare-kit-dotnet.%:
	rm -rf $@
	mkdir -p $(dir $@)
	cp -a $< $@

%: %-release.aar
	rm -rf $@
	mkdir -p $@
	unzip -q $< -d $@


all: ios/BareKit.xcframework android/bare-kit dotnet/runtime

ios/BareKit.xcframework:  ios/BareKit.framework ios-simulator/BareKit.framework

ios/BareKit.framework: ios-arm64/BareKit.framework

ios-simulator/BareKit.framework: ios-arm64-simulator/BareKit.framework ios-x64-simulator/BareKit.framework
