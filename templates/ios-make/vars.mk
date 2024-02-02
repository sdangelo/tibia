BUNDLE_NAME := {{=it.product.bundleName}}

PROJECT_NAME := {{=it.ios_make.projectName}}
TARGET_NAME := {{=it.ios_make.targetName}}

3RDPARTYFILES := {{=it.ios_make["3rdpartyfiles"].join(' ')}}

HAS_MIDI_IN := {{=it.product.buses.filter(x => x.type == "midi" && x.direction == "input").length > 0 ? "yes" : "no"}}
