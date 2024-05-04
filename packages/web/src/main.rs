use dioxus::prelude::*;
use wasm_bindgen::prelude::*;

#[wasm_bindgen(module = "/src/package.mjs")]
extern "C" {
    pub type Engine;

    #[wasm_bindgen(constructor)]
    pub fn new() -> Engine;

    #[wasm_bindgen(method)]
    pub fn test(this: &Engine);
}

fn app() -> Element {
    Engine::new().test();
    rsx! {
        div {
            "Hello, world!"
        }
    }
}

fn main() {
    launch(app);
}
