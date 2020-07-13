# Symlink to Adreno GLESv2 blobs
$(shell mkdir -p $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM)/lib/)
$(shell mkdir -p $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM)/lib64/)
$(shell pushd $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM)/lib/ > /dev/null && ln -s /vendor/lib/egl/libGLESv2_adreno.so libGLESv2_adreno.so && popd > /dev/null)
$(shell pushd $(PRODUCT_OUT)/$(TARGET_COPY_OUT_SYSTEM)/lib64/ > /dev/null && ln -s /vendor/lib64/egl/libGLESv2_adreno.so libGLESv2_adreno.so && popd > /dev/null)
