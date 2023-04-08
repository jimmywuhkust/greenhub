/****************************************************************************
MIT License

Copyright (c) 2017-2018 gdsports625@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************/
const char SKETCH_JS[] PROGMEM = R"=====(

//////////////////////////////////////////////
// ************** WebSocket *************** //
//////////////////////////////////////////////

var websocket;

function wsConnect() {
  
  // start a WebSocket
  websocket = new WebSocket('ws://' + window.location.hostname + ':81/');
  console.log("ws://" + window.location.hostname + ":81/");
    
  websocket.onopen = function(event) { onOpen(event) }
  websocket.onclose = function(event) { onClose(event) }
  websocket.onmessage = function(event) { onMessage(event) }
  websocket.onerror = function(event) { console.log(event.data); }
}

function onOpen(event){
  console.log('websocket opened ' + event.data);
}

function onClose(event){
  console.log('websocket closed ' + event.data);
  wsConnect();
}

function onMessage(event){
  // console.log('websocket data: ' + event.data);
  var value = String(event.data);
  value = parseInt(value);
  receivedData(value);
}

//////////////////////////////////////////////
// **************** P5.js ***************** //
//////////////////////////////////////////////

let system;
let lastSpawnTime = 0;
var spawnRate = 5

function setup() {
  createCanvas(720, 400);
  system = new ParticleSystem(createVector(width / 2, 50));
}

function draw() {
  background(255);
   const currentTime = millis();
  if (currentTime - lastSpawnTime > spawnRate) {
    system.addParticle();
    lastSpawnTime = currentTime;
  }
  system.run();
}

// A simple Particle class
let Particle = function(position) {
  this.acceleration = createVector(0, 0.05);
  this.velocity = createVector(random(-1, 1), random(-1, 0));
  this.position = position.copy();
  this.lifespan = 255;
};

Particle.prototype.run = function() {
  this.update();
  this.display();
};

// Method to update position
Particle.prototype.update = function(){
  this.velocity.add(this.acceleration);
  this.position.add(this.velocity);
  this.lifespan -= 2;
};

// Method to display
Particle.prototype.display = function() {
  stroke(0, 0, 255, this.lifespan);
  strokeWeight(2);
  fill(0, 127, 255, this.lifespan);
  ellipse(this.position.x, this.position.y, 12, 12);
};

// Is the particle still useful?
Particle.prototype.isDead = function(){
  return this.lifespan < 0;
};

let ParticleSystem = function(position) {
  this.origin = position.copy();
  this.particles = [];
};

ParticleSystem.prototype.addParticle = function() {
  this.particles.push(new Particle(this.origin));
};

ParticleSystem.prototype.run = function() {
  for (let i = this.particles.length-1; i >= 0; i--) {
    let p = this.particles[i];
    p.run();
    if (p.isDead()) {
      this.particles.splice(i, 1);
    }
  }
};

function receivedData(data) {
  var value = map(data, 0, 3300, 1, 300);
  spawnRate = value;
}

)=====";
