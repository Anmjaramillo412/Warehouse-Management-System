// ============================================================
// MODULE NAVIGATION
// ============================================================

function openModule(module) {

    const dashboard =
        document.querySelector(".dashboard");

    const moduleView =
        document.getElementById("module-view");

    const content =
        document.getElementById("module-content");

    dashboard.style.display = "none";

    moduleView.classList.remove("hidden");

    content.innerHTML = "";


    // ========================================================
    // MATERIAL MANAGER
    // ========================================================

    if (module === "materials") {

        content.innerHTML = `

            <h1>
                Material Manager
            </h1>

            <p>
                Manage materials stored in the system.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateMaterial()">
                    Create Material
                </button>

                <button onclick="showModifyMaterial()">
                    Modify Material
                </button>

                <button onclick="showDeleteMaterial()">
                    Delete Material
                </button>

                <button onclick="showSearchMaterial()">
                    Search Material
                </button>

                <button onclick="displayMaterials()">
                    Display Materials
                </button>

            </div>

            <div id="material-content">
            </div>
        `;
    }

    // ========================================================
    // PRODUCT MANAGER
    // ========================================================

    else if (module === "products") {

        content.innerHTML = `

            <h1>
                Product Manager
            </h1>

            <p>
                Manage products and their Bills of Materials.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateProduct()">
                    Create Product
                </button>

                <button onclick="showDisplayProducts()">
                    Display Products
                </button>

                <button onclick="showDeleteProduct()">
                    Delete Product
                </button>

                <button onclick="showSellProduct()">
                    Sell Product
                </button>

            </div>

            <div id="product-content">
            </div>

        `;
    }

    // ========================================================
    // WAREHOUSE MANAGER
    // ========================================================

    else if (module === "warehouses") {

        content.innerHTML = `

            <h1>
                Warehouse Manager
            </h1>

            <p>
                Manage warehouses and storage locations.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateWarehouse()">
                    Create Warehouse
                </button>

                <button onclick="showDeleteWarehouse()">
                    Delete Warehouse
                </button>

                <button onclick="displayWarehouses()">
                    Display Warehouses
                </button>

            </div>

            <div id="warehouse-content">
            </div>
        `;
    }


    // ========================================================
    // INVENTORY MANAGER
    // ========================================================

    else if (module === "inventory") {

        content.innerHTML = `

            <h1>
                Inventory Manager
            </h1>

            <p>
                Manage material movements and inventory.
            </p>

            <div class="module-buttons">

                <button onclick="showGoodsReceipt()">
                    Goods Receipt
                </button>

                <button onclick="showGoodsIssue()">
                    Goods Issue
                </button>

                <button onclick="showTransferMaterial()">
                    Transfer Material
                </button>

                <button onclick="displayInventory()">
                    Display Inventory
                </button>

                <button onclick="inventoryCheck()">
                    Inventory Check
                </button>

            </div>

            <div id="inventory-content">
            </div>
        `;
    }


    // ========================================================
    // DATA MANAGEMENT
    // ========================================================

    else if (module === "data") {

        content.innerHTML = `

            <h1>
                Data Management
            </h1>

            <p>
                Save and recover system data.
            </p>


            <div class="module-buttons">

                <button onclick="saveData()">
                    Save Data
                </button>

                <button onclick="loadData()">
                    Load Data
                </button>

            </div>


            <div class="data-logging-option">

                <label>

                    <input
                        type="checkbox"
                        id="log-data-operations"
                        onchange="setDataLogging()"
                    >

                    Log Save/Load operations

                </label>

            </div>


            <div id="data-logging-message">
            </div>

        `;


        loadDataLoggingState();
    }
}

// ============================================================
// LOAD DATA LOGGING STATE
// ============================================================

async function loadDataLoggingState()
{
    try
    {
        const response =
            await fetch(
                "/api/data/logging"
            );


        const data =
            await response.json();


        if (!response.ok)
        {
            return;
        }


        const checkbox =
            document.getElementById(
                "log-data-operations"
            );


        if (checkbox)
        {
            checkbox.checked =
                data.enabled;
        }
    }
    catch (error)
    {
        console.error(
            "Could not load logging state:",
            error
        );
    }
}

// ============================================================
// SAVE DATA
// ============================================================

async function saveData() {

    const message =
            document.getElementById(
                "data-logging-message"
            );

    try {

        const response =
            await fetch(
                "/api/data/save",
                {
                    method: "POST"
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "✓ Data saved successfully.";

        }
        else {

            message.textContent =
                "Error saving data: "
                + responseText;

        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// LOAD DATA
// ============================================================

async function loadData() {
    
    const message =
        document.getElementById(
            "data-logging-message"
        );

    try {

        const response =
            await fetch(
                "/api/data/load",
                {
                    method: "POST"
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {
            message.textContent =
                "✓ Data loaded successfully.";

        }
        else {

            message.textContent =
                "Error loading data: "
                + responseText;

        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// CREATE MATERIAL FORM
// ============================================================

function showCreateMaterial() {

    const content =
        document.getElementById("material-content");

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Material
            </h2>

            <label>
                Material ID
            </label>

            <input
                type="text"
                id="material-id"
                placeholder="###-######"
                maxlength="10"
            >

            <small>
                Format: ###-######
            </small>


            <label>
                Material Name
            </label>

            <input
                type="text"
                id="material-name"
                placeholder="Enter Material Name"
            >


            <label>
                Description
            </label>

            <textarea
                id="material-description"
                placeholder="Enter Material Description"
                rows="4"></textarea>


            <label>
                Unit of Measure
            </label>

            <input
                type="text"
                id="material-uom"
                value="pcs"
                placeholder="pcs"
            >


            <label>
                Category
            </label>

            <input
                type="text"
                id="material-category"
                placeholder="Enter Category"
            >


            <label>
                Supplier
            </label>

            <input
                type="text"
                id="material-supplier"
                placeholder="Enter Supplier"
            >


            <label>
                Material Photo
            </label>

            <input
                type="file"
                id="material-photo"
                accept="image/*"
            >

            <label class="checkbox-label">

                <input
                    type="checkbox"
                    id="material-active"
                    checked
                >

                Active Material

            </label>


            <div class="form-actions">

                <button
                    onclick="createMaterial()">

                    Create Material

                </button>

            </div>


            <div id="material-message">
            </div>

        </div>
    `;
}

// ============================================================
// CREATE MATERIAL
// ============================================================

async function createMaterial() {

    const id =
        document.getElementById(
            "material-id"
        ).value.trim();

    const name =
        document.getElementById(
            "material-name"
        ).value.trim();

    const description =
        document.getElementById(
            "material-description"
        ).value.trim();

    const uom =
        document.getElementById(
            "material-uom"
        ).value.trim();

    const category =
        document.getElementById(
            "material-category"
        ).value.trim();

    const supplier =
        document.getElementById(
            "material-supplier"
        ).value.trim();

    const active =
        document.getElementById(
            "material-active"
        ).checked;

    const photoInput =
        document.getElementById(
            "material-photo"
        );

    const photoFile =
        photoInput.files[0];

    let photoData = "";
    let photoName = "";

    if (photoFile) {

        photoName =
            photoFile.name;

        photoData =
            await readFileAsBase64(
                photoFile
            );
    }

    // ========================================================
    // VALIDATION
    // ========================================================

    const idPattern =
        /^[0-9]{3}-[0-9]{6}$/;

    const message =
        document.getElementById(
            "material-message"
        );


    if (!idPattern.test(id)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-######.";

        return;
    }


    if (!name ||
        !category ||
        !supplier) {

        message.textContent =
            "Please fill in all required fields.";

        return;
    }


    // ========================================================
    // PREPARE JSON
    // ========================================================

    const material = {

        id: id,

        name: name,

        description: description,

        uom: uom || "pcs",

        category: category,

        supplier: supplier,

        photo: photoName,

        photoData: photoData,   

        active: active
    };


    // ========================================================
    // SEND TO C++
    // ========================================================

    try {

        const response =
            await fetch(
                "/api/materials/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(material)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Material created successfully.";


            document.getElementById(
                "material-id"
            ).value = "";

            document.getElementById(
                "material-name"
            ).value = "";

            document.getElementById(
                "material-description"
            ).value = "";

            document.getElementById(
                "material-uom"
            ).value = "pcs";

            document.getElementById(
                "material-category"
            ).value = "";

            document.getElementById(
                "material-supplier"
            ).value = "";

            document.getElementById(
                "material-photo"
            ).value = "";

            document.getElementById(
                "material-active"
            ).checked = true;

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY MATERIALS
// ============================================================

async function displayMaterials() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `
        <p>Loading materials...</p>
    `;


    try {

        const response =
            await fetch(
                "/api/materials"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.materials ||
            data.materials.length === 0
        ) {

            content.innerHTML = `

                <div class="empty-message">

                    No materials available.

                </div>
            `;

            return;
        }


        let html = `

            <div class="material-table-container">

                <h2>
                    Materials
                </h2>

                <table class="material-table">

                    <thead>

                        <tr>

                            <th>Photo</th>    
                            <th>ID</th>
                            <th>Name</th>
                            <th>Description</th>
                            <th>UoM</th>
                            <th>Category</th>
                            <th>Supplier</th>
                            <th>Active</th>

                        </tr>

                    </thead>

                    <tbody>
        `;


        for (
            const material of data.materials
        ) {

            html += `

                <tr>

                    <td>

                        ${
                            material.photo
                            ? `<img
                                    src="/${material.photo}"
                                    class="material-thumbnail"
                                    alt="Material photo"
                               >`
                            : "No photo"
                        }

                    </td>

                    <td>
                        ${escapeHtml(material.id)}
                    </td>

                    <td>
                        ${escapeHtml(material.name)}
                    </td>

                    <td>
                        ${escapeHtml(material.description || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.uom || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.category || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.supplier || "")}
                    </td>

                    <td>
                        ${material.active ? "Yes" : "No"}
                    </td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

            </div>
        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// SEARCH MATERIAL FORM
// ============================================================

function showSearchMaterial() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Search Material
            </h2>

            <label>
                Material ID
            </label>

            <input
                type="text"
                id="search-material-id"
                placeholder="###-######"
                maxlength="10"
            >

            <div class="form-actions">

                <button
                    onclick="searchMaterial()">

                    Search Material

                </button>

            </div>

            <div id="search-material-result">
            </div>

        </div>
    `;
}


// ============================================================
// SEARCH MATERIAL
// ============================================================

async function searchMaterial() {

    const id =
        document.getElementById(
            "search-material-id"
        ).value.trim();


    const result =
        document.getElementById(
            "search-material-result"
        );


    const idPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    if (!idPattern.test(id)) {

        result.innerHTML = `
            <p>
                Invalid Material ID.
                Expected format ###-######.
            </p>
        `;

        return;
    }


    result.innerHTML = `
        <p>
            Searching...
        </p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            result.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }


        result.innerHTML = `

            <div class="search-result">

                <h3>
                    Material Found
                </h3>

                <p>
                    <strong>ID:</strong>
                    ${data.id}
                </p>

                <p>
                    <strong>Name:</strong>
                    ${data.name}
                </p>

                <p>
                    <strong>Description:</strong>
                    ${data.description}
                </p>

                <p>
                    <strong>Unit of Measure:</strong>
                    ${data.uom}
                </p>

                <p>
                    <strong>Category:</strong>
                    ${data.category}
                </p>

                <p>
                    <strong>Supplier:</strong>
                    ${data.supplier}
                </p>

                <p>
                    <strong>Active:</strong>
                    ${data.active ? "Yes" : "No"}
                </p>

            </div>
        `;

    }
    catch (error) {

        console.error(error);

        result.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}


// ============================================================
// MODIFY MATERIAL FORM
// ============================================================

function showModifyMaterial() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Modify Material
            </h2>

            <label>
                Material ID
            </label>

            <input
                type="text"
                id="modify-material-id"
                placeholder="###-######"
                maxlength="10"
            >

            <div class="form-actions">

                <button
                    onclick="loadMaterialForModify()">

                    Load Material

                </button>

            </div>

            <div id="modify-form">
            </div>

        </div>
    `;
}

// ============================================================
// DELETE MATERIAL FORM
// ============================================================

function showDeleteMaterial() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Material
            </h2>


            <label>
                Material ID
            </label>

            <input
                type="text"
                id="delete-material-id"
                placeholder="###-######"
                maxlength="10"
            >


            <div class="form-actions">

                <button
                    onclick="loadMaterialForDelete()">

                    Find Material

                </button>

            </div>


            <div id="delete-content">
            </div>

        </div>
    `;
}

// ============================================================
// LOAD MATERIAL FOR DELETE
// ============================================================

async function loadMaterialForDelete() {

    const id =
        document.getElementById(
            "delete-material-id"
        ).value.trim();


    const content =
        document.getElementById(
            "delete-content"
        );


    const idPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    if (!idPattern.test(id)) {

        content.innerHTML = `
            <p>
                Invalid Material ID.
                Expected format ###-######.
            </p>
        `;

        return;
    }


    content.innerHTML = `
        <p>
            Loading material...
        </p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            content.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }


        content.innerHTML = `

            <div class="search-result">

                <h3>
                    Material to Delete
                </h3>

                <p>
                    <strong>ID:</strong>
                    ${data.id}
                </p>

                <p>
                    <strong>Name:</strong>
                    ${escapeHtml(data.name)}
                </p>

                <p>
                    <strong>Description:</strong>
                    ${escapeHtml(data.description || "")}
                </p>

                <p>
                    <strong>Category:</strong>
                    ${escapeHtml(data.category)}
                </p>

                <p>
                    <strong>Supplier:</strong>
                    ${escapeHtml(data.supplier)}
                </p>


                <div class="delete-confirmation">

                    <p>
                        <strong>
                            Are you sure you want to delete
                            this material?
                        </strong>
                    </p>


                    <button
                        class="delete-button"
                        onclick="deleteMaterial(
                            '${data.id}'
                        )">

                        Yes, Delete Material

                    </button>


                    <button
                        class="cancel-button"
                        onclick="showDeleteMaterial()">

                        Cancel

                    </button>

                </div>

            </div>

        `;
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE MATERIAL
// ============================================================

async function deleteMaterial(id) {

    const content =
        document.getElementById(
            "material-content"
        );


    try {

        const response =
            await fetch(
                "/api/materials/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: id
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            content.innerHTML = `

                <div class="search-result">

                    <h3>
                        Material Deleted
                    </h3>

                    <p>
                        Material
                        <strong>${escapeHtml(id)}</strong>
                        was deleted successfully.
                    </p>

                </div>
            `;
        }
        else {

            content.innerHTML = `

                <div class="search-result">

                    <p>
                        Error: ${escapeHtml(responseText)}
                    </p>

                </div>
            `;
        }
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// ============================================================
// LOAD MATERIAL FOR MODIFY
// ============================================================

async function loadMaterialForModify() {

    const id =
        document.getElementById(
            "modify-material-id"
        ).value.trim();


    const modifyForm =
        document.getElementById(
            "modify-form"
        );


    const idPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    if (!idPattern.test(id)) {

        modifyForm.innerHTML = `
            <p>
                Invalid Material ID.
                Expected format ###-######.
            </p>
        `;

        return;
    }


    modifyForm.innerHTML = `
        <p>
            Loading material...
        </p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            modifyForm.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }


        modifyForm.innerHTML = `

            <div class="form-container">

                <h2>
                    Modify Material
                </h2>


                <p>
                    Material ID:
                    <strong>${data.id}</strong>
                </p>


                <label>
                    Material Name
                </label>

                <input
                    type="text"
                    id="modify-name"
                    value="${escapeHtml(data.name)}"
                >


                <label>
                    Description
                </label>

                <textarea
                    id="modify-description"
                    rows="4"
                >${escapeHtml(data.description || "")}</textarea>


                <label>
                    Unit of Measure
                </label>

                <input
                    type="text"
                    id="modify-uom"
                    value="${escapeHtml(data.uom)}"
                >


                <label>
                    Category
                </label>

                <input
                    type="text"
                    id="modify-category"
                    value="${escapeHtml(data.category)}"
                >


                <label>
                    Supplier
                </label>

                <input
                    type="text"
                    id="modify-supplier"
                    value="${escapeHtml(data.supplier)}"
                >

                <label>
                    Material Photo
                </label>

                <input
                    type="file"
                    id="modify-photo-input"
                    accept="image/*"
                >

                <input
                    type="hidden"
                    id="modify-photo"
                    value="${escapeHtml(data.photo || "")}"
                >

                <label class="checkbox-label">

                    <input
                        type="checkbox"
                        id="modify-active"
                        ${data.active ? "checked" : ""}
                    >

                    Active Material

                </label>


                <div class="form-actions">

                    <button
                        onclick="modifyMaterial()">

                        Save Changes

                    </button>

                </div>


                <div id="modify-message">
                </div>

            </div>

        `;
    }
    catch (error) {

        console.error(error);

        modifyForm.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// HTML ESCAPE
// ============================================================

function escapeHtml(value) {

    return String(value)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

// ============================================================
// MODIFY MATERIAL
// ============================================================

async function modifyMaterial() {

    const id =
        document.getElementById(
            "modify-material-id"
        ).value.trim();

    const name =
        document.getElementById(
            "modify-name"
        ).value.trim();

    const description =
        document.getElementById(
            "modify-description"
        ).value.trim();

    const uom =
        document.getElementById(
            "modify-uom"
        ).value.trim();

    const category =
        document.getElementById(
            "modify-category"
        ).value.trim();

    const supplier =
        document.getElementById(
            "modify-supplier"
        ).value.trim();

    const active =
        document.getElementById(
            "modify-active"
        ).checked;

    const photoInput =
        document.getElementById(
            "modify-photo-input"
        );

    const photoFile =
        photoInput.files[0];

    let photoPath =
        document.getElementById(
            "modify-photo"
        ).value;


    let photoData = "";


    if (photoFile)
    {
        photoData =
            await readFileAsBase64(
                photoFile
            );
    }

    if (!name ||
        !category ||
        !supplier) {

        message.textContent =
            "Please fill in all required fields.";

        return;
    }


    const material = {

        id: id,

        name: name,

        description: description,

        uom: uom || "pcs",

        category: category,

        supplier: supplier,

        photo: photoPath,

        photoData: photoData,

        active: active
    };


    try {

        const response =
            await fetch(
                "/api/materials/modify",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(material)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Material successfully modified.";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// CREATE WAREHOUSE FORM
// ============================================================

function showCreateWarehouse() {

    const content =
        document.getElementById(
            "warehouse-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Warehouse
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <label>
                Warehouse Name
            </label>

            <input
                type="text"
                id="warehouse-name"
                placeholder="Enter Warehouse Name"
            >


            <div class="form-actions">

                <button
                    onclick="createWarehouse()">

                    Create Warehouse

                </button>

            </div>


            <div id="warehouse-message">
            </div>

        </div>
    `;
}

// ============================================================
// CREATE WAREHOUSE
// ============================================================

async function createWarehouse() {

    const id =
        document.getElementById(
            "warehouse-id"
        ).value;

    const name =
        document.getElementById(
            "warehouse-name"
        ).value.trim();


    const message =
        document.getElementById(
            "warehouse-message"
        );


    if (!id || !name) {

        message.textContent =
            "Please fill in all required fields.";

        return;
    }


    const warehouse = {

        id: Number(id),

        name: name

    };


    try {

        const response =
            await fetch(
                "/api/warehouses/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(warehouse)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Warehouse created successfully.";


            document.getElementById(
                "warehouse-id"
            ).value = "";

            document.getElementById(
                "warehouse-name"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY WAREHOUSES
// ============================================================

async function displayWarehouses() {

    const content =
        document.getElementById(
            "warehouse-content"
        );


    content.innerHTML = `
        <p>
            Loading warehouses...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/warehouses"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.warehouses ||
            data.warehouses.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">
                    No warehouses available.
                </div>
            `;

            return;
        }


        let html = `

            <div class="material-table-container">

                <h2>
                    Warehouses
                </h2>

                <table class="material-table">

                    <thead>

                        <tr>

                            <th>Warehouse ID</th>
                            <th>Warehouse Name</th>

                        </tr>

                    </thead>

                    <tbody>
        `;


        for (
            const warehouse of data.warehouses
        ) {

            html += `

                <tr>

                    <td>
                        ${warehouse.id}
                    </td>

                    <td>
                        ${escapeHtml(
                            warehouse.name
                        )}
                    </td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

            </div>
        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE WAREHOUSE FORM
// ============================================================

function showDeleteWarehouse() {

    const content =
        document.getElementById(
            "warehouse-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Warehouse
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="delete-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <div class="form-actions">

                <button
                    onclick="deleteWarehouse()">

                    Delete Warehouse

                </button>

            </div>


            <div id="warehouse-delete-message">
            </div>

        </div>
    `;
}

// ============================================================
// DELETE WAREHOUSE
// ============================================================

async function deleteWarehouse() {

    const id =
        document.getElementById(
            "delete-warehouse-id"
        ).value;


    const message =
        document.getElementById(
            "warehouse-delete-message"
        );


    if (!id) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    try {

        const response =
            await fetch(
                "/api/warehouses/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: Number(id)
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Warehouse deleted successfully.";


            document.getElementById(
                "delete-warehouse-id"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// GOODS RECEIPT FORM
// ============================================================

function showGoodsReceipt() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Goods Receipt
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="receipt-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <label>
                Material ID
            </label>

            <input
                type="text"
                id="receipt-material-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Quantity
            </label>

            <input
                type="number"
                id="receipt-quantity"
                min="1"
                placeholder="Enter Quantity"
            >

            <label>
                Comment
            </label>

            <textarea
                id="receipt-comment"
                rows="4"
                placeholder="Reason or additional information"></textarea>


            <div class="form-actions">

                <button
                    onclick="goodsReceipt()">

                    Receive Goods

                </button>

            </div>


            <div id="receipt-message">
            </div>

        </div>
    `;
}

// ============================================================
// GOODS RECEIPT
// ============================================================

async function goodsReceipt() {

    const warehouseID =
        document.getElementById(
            "receipt-warehouse-id"
        ).value;

    const materialID =
        document.getElementById(
            "receipt-material-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "receipt-quantity"
        ).value;

    const comment =
        document.getElementById(
            "receipt-comment"
        ).value.trim();

    const message =
        document.getElementById(
            "receipt-message"
        );


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    const materialIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    if (!materialIDPattern.test(materialID)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-######.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    // --------------------------------------------------------
    // Prepare request
    // --------------------------------------------------------

    const receipt = {

        warehouseID:
            Number(warehouseID),

        materialID:
            materialID,

        quantity:
            Number(quantity),

        comment:
            comment

    };


    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/inventory/receipt",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(receipt)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Goods receipt completed successfully.";

            clearInputFields([
                "receipt-warehouse-id",
                "receipt-material-id",
                "receipt-quantity",
                "receipt-comment"
            ]);

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// GOODS ISSUE FORM
// ============================================================

function showGoodsIssue() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Goods Issue
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="issue-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <label>
                Material ID
            </label>

            <input
                type="text"
                id="issue-material-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Quantity
            </label>

            <input
                type="number"
                id="issue-quantity"
                min="1"
                placeholder="Enter Quantity"
            >

            <label>
                Comment
            </label>

            <textarea
                id="issue-comment"
                rows="4"
                placeholder="Reason or additional information"></textarea>


            <div class="form-actions">

                <button
                    onclick="goodsIssue()">

                    Issue Goods

                </button>

            </div>


            <div id="issue-message">
            </div>

        </div>
    `;
}

// ============================================================
// GOODS ISSUE
// ============================================================

async function goodsIssue() {

    const warehouseID =
        document.getElementById(
            "issue-warehouse-id"
        ).value;

    const materialID =
        document.getElementById(
            "issue-material-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "issue-quantity"
        ).value;

    const comment =
        document.getElementById(
            "issue-comment"
        ).value.trim();


    const message =
        document.getElementById(
            "issue-message"
        );


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    const materialIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    if (!materialIDPattern.test(materialID)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-######.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    // --------------------------------------------------------
    // Prepare request
    // --------------------------------------------------------

    const issue = {

        warehouseID:
            Number(warehouseID),

        materialID:
            materialID,

        quantity:
            Number(quantity),

        comment:
            comment
    };


    // --------------------------------------------------------
    // Send request
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/inventory/issue",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(issue)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Goods issue completed successfully.";

            clearInputFields([
                "issue-warehouse-id",
                "issue-material-id",
                "issue-quantity",
                "issue-comment"
            ]);

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// TRANSFER MATERIAL FORM
// ============================================================

function showTransferMaterial() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Transfer Material
            </h2>


            <label>
                Source Warehouse ID
            </label>

            <input
                type="number"
                id="transfer-source"
                placeholder="Enter Source Warehouse ID"
            >


            <label>
                Destination Warehouse ID
            </label>

            <input
                type="number"
                id="transfer-destination"
                placeholder="Enter Destination Warehouse ID"
            >


            <label>
                Material ID
            </label>

            <input
                type="text"
                id="transfer-material-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Quantity
            </label>

            <input
                type="number"
                id="transfer-quantity"
                min="1"
                placeholder="Enter Quantity"
            >

            <label>
                Comment
            </label>

            <textarea
                id="transfer-comment"
                rows="4"
                placeholder="Reason or additional information"></textarea>

            <div class="form-actions">

                <button
                    onclick="transferMaterial()">

                    Transfer Material

                </button>

            </div>


            <div id="transfer-message">
            </div>

        </div>
    `;
}

// ============================================================
// TRANSFER MATERIAL
// ============================================================

async function transferMaterial() {

    const sourceWarehouseID =
        document.getElementById(
            "transfer-source"
        ).value;

    const destinationWarehouseID =
        document.getElementById(
            "transfer-destination"
        ).value;

    const materialID =
        document.getElementById(
            "transfer-material-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "transfer-quantity"
        ).value;

    const comment =
        document.getElementById(
            "transfer-comment"
        ).value.trim();

    const message =
        document.getElementById(
            "transfer-message"
        );

    const materialIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!sourceWarehouseID ||
        !destinationWarehouseID) {

        message.textContent =
            "Please enter both warehouse IDs.";

        return;
    }


    if (
        Number(sourceWarehouseID) ===
        Number(destinationWarehouseID)
    ) {

        message.textContent =
            "Source and destination warehouses must be different.";

        return;
    }


    if (!materialIDPattern.test(materialID)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-######.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    const transfer = {

        sourceWarehouseID:
            Number(sourceWarehouseID),

        destinationWarehouseID:
            Number(destinationWarehouseID),

        materialID:
            materialID,

        quantity:
            Number(quantity),

        comment:
            comment
    };

    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/inventory/transfer",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(transfer)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Material transferred successfully.";

            clearInputFields([
                "transfer-source",
                "transfer-destination",
                "transfer-material-id",
                "transfer-quantity",
                "transfer-comment"
            ]);
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY INVENTORY
// ============================================================

async function displayInventory() {

    const content =
        document.getElementById(
            "inventory-content"
        );


    content.innerHTML = `
        <p>
            Loading inventory...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/inventory"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.warehouses ||
            data.warehouses.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">

                    No inventory available.

                </div>
            `;

            return;
        }


        let html = "";


        for (
            const warehouse of data.warehouses
        ) {

            html += `

                <div class="material-table-container">

                    <h2>
                        Warehouse
                        ${warehouse.id}
                        -
                        ${escapeHtml(
                            warehouse.name
                        )}
                    </h2>
            `;


            if (
                !warehouse.inventory ||
                warehouse.inventory.length === 0
            ) {

                html += `

                    <div class="empty-message">

                        Warehouse is empty.

                    </div>

                `;

            }
            else {

                html += `

                    <table class="material-table">

                        <thead>

                            <tr>

                                <th>
                                    Material ID
                                </th>

                                <th>
                                    Material Name
                                </th>

                                <th>
                                    Quantity
                                </th>

                                <th>
                                    UoM
                                </th>

                            </tr>

                        </thead>

                        <tbody>
                `;


                for (
                    const item of warehouse.inventory
                ) {

                    html += `

                        <tr>

                            <td>
                                ${escapeHtml(
                                    item.materialID
                                )}
                            </td>

                            <td>
                                ${escapeHtml(
                                    item.materialName
                                )}
                            </td>

                            <td>
                                ${item.quantity}
                            </td>

                            <td>
                                ${escapeHtml(
                                    item.uom
                                )}
                            </td>

                        </tr>

                    `;
                }


                html += `

                        </tbody>

                    </table>

                `;
            }


            html += `

                </div>

            `;
        }


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// ============================================================
// INVENTORY CHECK
// ============================================================

async function inventoryCheck() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `
        <p>
            Checking inventory...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/inventory/check"
            );


        const data =
            await response.json();


        if (!response.ok) {

            content.innerHTML = `

                <div class="empty-message">

                    Inventory check failed.

                    <br><br>

                    ${escapeHtml(
                        data.message || ""
                    )}

                </div>

            `;

            return;
        }


        // ----------------------------------------------------
        // Inventory OK
        // ----------------------------------------------------

        if (data.valid) {

            content.innerHTML = `

                <div class="search-result">

                    <h3>
                        ✓ Inventory Check Passed
                    </h3>

                    <p>
                        No inconsistencies were found.
                    </p>

                    <p>
                        Warehouses checked:
                        <strong>
                            ${data.warehousesChecked}
                        </strong>
                    </p>

                    <p>
                        Inventory entries checked:
                        <strong>
                            ${data.entriesChecked}
                        </strong>
                    </p>

                </div>

            `;

            return;
        }


        // ----------------------------------------------------
        // Problems found
        // ----------------------------------------------------

        let html = `

            <div class="search-result">

                <h3>
                    Inventory Check
                </h3>

                <p>
                    The following inconsistencies were found:
                </p>

                <ul>
        `;


        for (
            const problem of data.problems
        ) {

            html += `

                <li>
                    ${escapeHtml(problem)}
                </li>

            `;
        }


        html += `

                </ul>

            </div>

        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// ============================================================
// CREATE PRODUCT FORM
// ============================================================

function showCreateProduct() {

    const content =
        document.getElementById(
            "product-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Product
            </h2>


            <label>
                Product ID
            </label>

            <input
                type="text"
                id="product-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Product Name
            </label>

            <input
                type="text"
                id="product-name"
                placeholder="Enter Product Name"
            >


            <label>
                Description
            </label>

            <textarea
                id="product-description"
                rows="4"
                placeholder="Enter Product Description"></textarea>


            <h3>
                Bill of Materials
            </h3>


            <div id="bom-items">
            </div>


            <button
                type="button"
                onclick="addBOMItemRow()">

                + Add Component

            </button>


            <div class="form-actions">

                <button
                    onclick="createProduct()">

                    Create Product

                </button>

            </div>


            <div id="product-message">
            </div>

        </div>
    `;
}

// ============================================================
// ADD BOM ITEM
// ============================================================

function addBOMItemRow() {

    const container =
        document.getElementById(
            "bom-items"
        );


    const row =
        document.createElement(
            "div"
        );

    row.className =
        "bom-row";


    row.innerHTML = `

        <input
            type="text"
            class="bom-material-id"
            placeholder="Material ID"
            maxlength="10"
        >


        <input
            type="number"
            class="bom-quantity"
            min="1"
            placeholder="Quantity"
        >


        <button
            type="button"
            onclick="this.parentElement.remove()">

            Remove

        </button>

    `;


    container.appendChild(row);
}

// ============================================================
// CREATE PRODUCT
// ============================================================

async function createProduct() {

    const id =
        document.getElementById(
            "product-id"
        ).value.trim();

    const name =
        document.getElementById(
            "product-name"
        ).value.trim();

    const description =
        document.getElementById(
            "product-description"
        ).value.trim();


    const message =
        document.getElementById(
            "product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    // --------------------------------------------------------
    // Validate Product ID
    // --------------------------------------------------------

    if (!productIDPattern.test(id)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    // --------------------------------------------------------
    // Validate required fields
    // --------------------------------------------------------

    if (!name) {

        message.textContent =
            "Product Name is required.";

        return;
    }


    // --------------------------------------------------------
    // Read BOM rows
    // --------------------------------------------------------

    const rows =
        document.querySelectorAll(
            ".bom-row"
        );


    const bom = [];


    for (const row of rows) {

        const materialID =
            row.querySelector(
                ".bom-material-id"
            ).value.trim();


        const quantity =
            Number(
                row.querySelector(
                    ".bom-quantity"
                ).value
            );


        if (!/^[0-9]{3}-[0-9]{6}$/.test(materialID))
        {
            message.textContent =
                "Invalid Material ID in BOM.";

            return;
        }


        if (quantity <= 0)
        {
            message.textContent =
                "BOM quantity must be greater than zero.";

            return;
        }


        bom.push({

            materialID:
                materialID,

            quantity:
                quantity

        });
    }


    // --------------------------------------------------------
    // Create product object
    // --------------------------------------------------------

    const product = {

        id: id,

        name: name,

        description: description,

        bom: bom

    };


    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/products/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(product)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product created successfully.";

            clearInputFields([
                "product-id",
                "product-name",
                "product-description"
            ]);
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY PRODUCTS
// ============================================================

async function showDisplayProducts() {

    const content =
        document.getElementById(
            "product-content"
        );


    content.innerHTML = `
        <p>
            Loading products...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/products"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${escapeHtml(message)}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.products ||
            data.products.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">
                    No products available.
                </div>
            `;

            return;
        }


        let html = "";


        for (
            const product of data.products
        ) {

            html += `

                <div class="material-table-container">

                    <h2>
                        ${escapeHtml(product.id)}
                        -
                        ${escapeHtml(product.name)}
                    </h2>

                    <p>
                        ${escapeHtml(
                            product.description || ""
                        )}
                    </p>
            `;


            if (
                !product.bom ||
                product.bom.length === 0
            ) {

                html += `

                    <div class="empty-message">
                        No BOM components.
                    </div>

                `;

            }
            else {

                html += `

                    <table class="material-table">

                        <thead>

                            <tr>

                                <th>
                                    Material ID
                                </th>

                                <th>
                                    Quantity
                                </th>

                            </tr>

                        </thead>

                        <tbody>
                `;


                for (
                    const item of product.bom
                ) {

                    html += `

                        <tr>

                            <td>
                                ${escapeHtml(
                                    item.materialID
                                )}
                            </td>

                            <td>
                                ${item.quantity}
                            </td>

                        </tr>

                    `;
                }


                html += `

                        </tbody>

                    </table>

                `;
            }


            html += `
                </div>
            `;
        }


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE PRODUCT FORM
// ============================================================

function showDeleteProduct() {

    const content =
        document.getElementById(
            "product-content"
        );


    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Product
            </h2>


            <label>
                Product ID
            </label>

            <input
                type="text"
                id="delete-product-id"
                placeholder="###-######"
                maxlength="10"
            >


            <div class="form-actions">

                <button
                    onclick="deleteProduct()">

                    Delete Product

                </button>

            </div>


            <div id="delete-product-message">
            </div>

        </div>
    `;
}

// ============================================================
// DELETE PRODUCT
// ============================================================

async function deleteProduct() {

    const id =
        document.getElementById(
            "delete-product-id"
        ).value.trim();


    const message =
        document.getElementById(
            "delete-product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    if (!productIDPattern.test(id)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    try {

        const response =
            await fetch(
                "/api/products/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: id
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product deleted successfully.";

            document.getElementById(
                "delete-product-id"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// SELL PRODUCT FORM
// ============================================================

function showSellProduct() {

    const content =
        document.getElementById(
            "product-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Sell Product
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="sell-warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <label>
                Product ID
            </label>

            <input
                type="text"
                id="sell-product-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Quantity Sold
            </label>

            <input
                type="number"
                id="sell-quantity"
                min="1"
                placeholder="Enter Quantity"
            >


            <div class="form-actions">

                <button
                    onclick="sellProduct()">

                    Sell Product

                </button>

            </div>


            <div id="sell-product-message">
            </div>

        </div>
    `;
}

// ============================================================
// SELL PRODUCT
// ============================================================

async function sellProduct() {

    const warehouseID =
        document.getElementById(
            "sell-warehouse-id"
        ).value;

    const productID =
        document.getElementById(
            "sell-product-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "sell-quantity"
        ).value;


    const message =
        document.getElementById(
            "sell-product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    if (!productIDPattern.test(productID)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    const sale = {

        warehouseID:
            Number(warehouseID),

        productID:
            productID,

        quantity:
            Number(quantity)

    };


    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/products/sell",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(sale)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product sold successfully.";

            document.getElementById(
                "sell-quantity"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// READ FILE AS BASE64
// ============================================================

function readFileAsBase64(file) {

    return new Promise(
        (resolve, reject) => {

            const reader =
                new FileReader();

            reader.onload = () => {

                resolve(
                    reader.result
                );
            };

            reader.onerror = () => {

                reject(
                    new Error(
                        "Could not read image file."
                    )
                );
            };

            reader.readAsDataURL(file);
        }
    );
}

// ============================================================
// CLEAR INPUT FIELDS
// ============================================================

function clearInputFields(ids) {

    ids.forEach(id => {

        const element =
            document.getElementById(id);

        if (element) {

            element.value = "";

        }
    });
}

// ============================================================
// SET DATA LOGGING
// ============================================================

async function setDataLogging() {

    const checkbox =
        document.getElementById(
            "log-data-operations"
        );

    const message =
        document.getElementById(
            "data-logging-message"
        );


    try {

        const response =
            await fetch(
                "/api/data/logging",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            enabled:
                                checkbox.checked
                        })
                }
            );


        const data =
            await response.json();


        if (!response.ok) {

            message.textContent =
                data.message ||
                "Could not update logging setting.";

            return;
        }


        message.textContent =
            data.message;

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// BACK TO DASHBOARD
// ============================================================

function closeModule() {

    const dashboard =
        document.querySelector(
            ".dashboard"
        );

    const moduleView =
        document.getElementById(
            "module-view"
        );


    moduleView.classList.add(
        "hidden"
    );

    dashboard.style.display =
        "grid";
}