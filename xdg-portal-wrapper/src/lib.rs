use std::{ffi::CString, os::raw::c_char};

use ashpd::{
    desktop::file_chooser::{FileFilter, SelectedFiles},
    zvariant::NoneValue,
};
use async_compat::Compat;
use smol::Executor;

#[unsafe(no_mangle)]
pub unsafe extern "C" fn save_as() -> *mut c_char {
    // Open the save dialogue
    let files = run_sync(async || {
        SelectedFiles::save_file()
            .title("Save your painting")
            .accept_label("Save")
            .current_name("image.png")
            .modal(true)
            .filter(FileFilter::new("PNG Image").glob("*.png"))
            .send()
            .await
            .unwrap()
            .response()
            .unwrap_or_default()
    });

    // If the user picked a path, format it and return
    if let Some(path) = files.uris().first().map(|uri| uri.to_string()) {
        let path = path.trim_start_matches("file://");
        let cstr = CString::new(path).unwrap();
        return cstr.into_raw();
    }

    // If the user canceled, return empty string
    return CString::default().into_raw();
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn drop_rstring(ptr: *mut c_char) {
    if !ptr.is_null() {
        drop(unsafe { CString::from_raw(ptr) });
    }
}

// Many rust libs are async by default. This blocks them until finished
fn run_sync<Fut, T>(callback: impl Fn() -> Fut) -> T
where
    Fut: Future<Output = T> + Send,
    T: Send,
{
    let ex = Executor::new();
    let task = ex.spawn(Compat::new(callback()));
    smol::block_on(ex.run(task))
}
