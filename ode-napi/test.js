const ode = require("../build/"+(process.argv[2] || 'default')+"/ode-napi/ode-napi.node");
const util = require('util')

console.log(ode);
//ode.MemoryBuffer.prototype[util.inspect.custom] = function (depth, options, inspect) {
//    return `${options.stylize('MemoryBuffer', 'special')}<${this.length}>`;
//}
//
//console.log(ode.hello());
//const buffer = new ode.MemoryBuffer(0)
//const v = Uint8Array.from([1,2,3]).buffer
//buffer.set(v)
//console.log(buffer)

const engine = new ode.EngineHandle()
console.log('createEngine',ode.createEngine(engine, {}))
const design = new ode.DesignHandle()
console.log('createDesign', ode.createDesign(engine, design))
const component = new ode.ComponentHandle()
const octopus = ode.makeString(`{"type":"OCTOPUS_COMPONENT","version":"3.0.0-alpha.33","id":"OCTOPUS1","dimensions":{"width":640,"height":480},"content":{"id":"ROOT1","type":"GROUP","name":"Root","visible":true,"opacity":1,"blendMode":"NORMAL","transform":[1,0,0,1,0,0],"layers":[{"id":"SHAPE1","type":"SHAPE","name":"Shape 1","visible":true,"opacity":1,"blendMode":"NORMAL","transform":[1,0,0,1,0,0],"shape":{"path":{"type":"PATH","geometry":"M 193.287149 3.393170 L 476.025269 9.660451 L 482.682699 213.904965 L 192.292978 221.465743 Z","transform":[1,0,0,1,-100,100],"visible":true},"fills":[{"type":"COLOR","visible":true,"blendMode":"NORMAL","color":{"r":1,"g":0.5,"b":0,"a":1}}],"strokes":[{"fill":{"type":"COLOR","visible":true,"blendMode":"NORMAL","color":{"r":0.25,"g":0.25,"b":0.25,"a":1}},"thickness":6,"position":"OUTSIDE","visible":true,"style":"SOLID"}]},"effects":[{"type":"DROP_SHADOW","basis":"BODY","visible":true,"blendMode":"NORMAL","shadow":{"offset":{"x":20,"y":40},"blur":0,"choke":0,"color":{"r":0,"g":0,"b":0,"a":0.5}}}]},{"id":"SHAPE2","type":"SHAPE","name":"Shape 2","visible":true,"opacity":1,"blendMode":"NORMAL","transform":[1,0,0,1,0,0],"shape":{"path":{"type":"PATH","geometry":"M 283.078463 53.153966 L 563.101657 46.649861 L 568.693136 251.248390 L 277.516282 252.142949 Z","transform":[1,0,0,1,0,0],"visible":true},"fills":[{"type":"COLOR","visible":true,"blendMode":"NORMAL","color":{"r":0,"g":0.5,"b":1,"a":1}}],"strokes":[{"fill":{"type":"COLOR","visible":true,"blendMode":"NORMAL","color":{"r":0.25,"g":0.25,"b":0.25,"a":1}},"thickness":6,"position":"CENTER","visible":true,"style":"SOLID"}]},"effects":[{"type":"BLUR","basis":"LAYER_AND_EFFECTS","blur":4}]}],"effects":[]}}`)
console.log(octopus)
console.log('design_addComponentFromOctopusString', ode.design_addComponentFromOctopusString(design, component, {
    id: ode.makeString("OCTOPUS1"),
    page: ode.makeString("page"),
    position: [0,0],
}, octopus))
console.log(design)
