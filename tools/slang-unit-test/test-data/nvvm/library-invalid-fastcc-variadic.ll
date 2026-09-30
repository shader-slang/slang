; ModuleID = 'device-library-signature-control'
source_filename = "device-library-signature-control"

define fastcc float @__nv_roundf(float %value, ...) {
entry:
  ret float %value
}
